// Fill out your copyright notice in the Description page of Project Settings.

#include "Game/Lesson12.09/ObjectPoolSubsystem.h"

#include "Engine/World.h"
#include "Stats/Stats.h"
#include "UObject/UObjectGlobals.h"

#pragma region STATS

DECLARE_STATS_GROUP(
    TEXT("ObjectPool"),
    STATGROUP_ObjectPool,
    STATCAT_Advanced
);

DECLARE_DWORD_COUNTER_STAT(
    TEXT("Total Pools"),
    STAT_ObjectPool_TotalPools,
    STATGROUP_ObjectPool
);

DECLARE_DWORD_COUNTER_STAT(
    TEXT("Active Objects"),
    STAT_ObjectPool_ActiveObjects,
    STATGROUP_ObjectPool
);

DECLARE_DWORD_COUNTER_STAT(
    TEXT("Free Objects"),
    STAT_ObjectPool_FreeObjects,
    STATGROUP_ObjectPool
);

#pragma endregion   

#pragma region SUBSYSTEM IMPLEMENTATION

bool UObjectPoolSubsystem::DoesClassImplementInterface(TSubclassOf<AActor> ClassPool) const
{
    if (!IsClassValid(ClassPool))
    {
        return false;
    }

    // Check if the class implements the ObjectPoolInterface
    return ClassPool->ImplementsInterface(UObjectPoolInterface::StaticClass());
}

bool UObjectPoolSubsystem::IsClassValid(TSubclassOf<AActor> ClassPool) const
{
    return ClassPool != nullptr && ClassPool.Get() != nullptr;
}

void UObjectPoolSubsystem::AddPool(const TSubclassOf<AActor> ClassPool, int32 InitialSize)
{
    if (!GetWorld())
    {
        return;
    }

    // Check if class is valid
    if (!IsClassValid(ClassPool))
    {
        UE_LOG(LogTemp, Warning, TEXT("UObjectPoolSubsystem::AddPool - Invalid class provided"));
        return;
    }

    // Check if class implements the interface
    if (!DoesClassImplementInterface(ClassPool))
    {
        UE_LOG(LogTemp, Warning, TEXT("UObjectPoolSubsystem::AddPool - Class %s does not implement IObjectPoolInterface"), 
            *GetNameSafe(ClassPool.Get()));
        return;
    }

    FObjectPool ObjectPoolToCreate;

    FActorSpawnParameters SpawnParameters;
    SpawnParameters.SpawnCollisionHandlingOverride =
        ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    for (int32 i = 0; i < InitialSize; ++i)
    {
        AActor* ActorRef = GetWorld()->SpawnActor<AActor>(ClassPool, SpawnParameters);
        
        if (ActorRef && ActorRef->Implements<UObjectPoolInterface>())
        {
            // Calculate position in the pool grid
            int32 GridX = i % 10; // 10 objects per row
            int32 GridY = i / 10; // Row number
            FVector PoolPosition = PoolVisualizationLocation + FVector(
                GridX * PoolGridSpacing,
                GridY * PoolGridSpacing,
                0.0f
            );

            // Set actor location to pool position
            ActorRef->SetActorLocation(PoolPosition);
            
            // Keep visible but inactive
            ActorRef->SetActorHiddenInGame(false);
            ActorRef->SetActorEnableCollision(false);

            ObjectPoolToCreate.UsablePoolingObjects.AddUnique(ActorRef);
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("UObjectPoolSubsystem::AddPool - Failed to spawn or actor does not implement interface"));
            if (ActorRef)
            {
                ActorRef->Destroy();
            }
        }
    }

    ObjectPoolMap.Add(ClassPool, ObjectPoolToCreate);

    UpdateStats();
}

TScriptInterface<IObjectPoolInterface> UObjectPoolSubsystem::GetObjectFromPool(const TSubclassOf<AActor> ClassPool)
{
    if (!GetWorld())
    {
        return nullptr;
    }

    // Check if class is valid
    if (!IsClassValid(ClassPool))
    {
        UE_LOG(LogTemp, Warning, TEXT("UObjectPoolSubsystem::GetObjectFromPool - Invalid class provided"));
        return nullptr;
    }

    // Check if class implements the interface
    if (!DoesClassImplementInterface(ClassPool))
    {
        UE_LOG(LogTemp, Warning, TEXT("UObjectPoolSubsystem::GetObjectFromPool - Class %s does not implement IObjectPoolInterface"), 
            *GetNameSafe(ClassPool.Get()));
        return nullptr;
    }

    if (FObjectPool* PoolObject = ObjectPoolMap.Find(ClassPool))
    {
        TScriptInterface<IObjectPoolInterface> ActorToReturn = nullptr;

        if (PoolObject->UsablePoolingObjects.IsEmpty())
        {
            FActorSpawnParameters SpawnParameters;
            SpawnParameters.SpawnCollisionHandlingOverride =
                ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

            AActor* SpawnedActor = GetWorld()->SpawnActor<AActor>(ClassPool, SpawnParameters);
            
            if (SpawnedActor && SpawnedActor->Implements<UObjectPoolInterface>())
            {
                ActorToReturn = SpawnedActor;
            }
            else
            {
                UE_LOG(LogTemp, Warning, TEXT("UObjectPoolSubsystem::GetObjectFromPool - Spawned actor does not implement interface"));
                if (SpawnedActor)
                {
                    SpawnedActor->Destroy();
                }
                return nullptr;
            }
        }
        else
        {
            ActorToReturn = PoolObject->UsablePoolingObjects[0];
            PoolObject->UsablePoolingObjects.RemoveAtSwap(0);
        }

        if (ActorToReturn.GetObject())
        {
            PoolObject->ActivePoolingObjects.AddUnique(ActorToReturn);
            UpdateStats();
            return ActorToReturn;
        }
    }

    return nullptr;
}

void UObjectPoolSubsystem::ReturnObjectToPool(
    const TSubclassOf<AActor> ClassPool,
    const TScriptInterface<IObjectPoolInterface> ActorToReturn)
{
    if (!GetWorld())
    {
        return;
    }

    // Check if class is valid
    if (!IsClassValid(ClassPool))
    {
        UE_LOG(LogTemp, Warning, TEXT("UObjectPoolSubsystem::ReturnObjectToPool - Invalid class provided"));
        return;
    }

    // Check if actor is valid and implements interface
    if (!ActorToReturn.GetObject() || !ActorToReturn.GetObject()->Implements<UObjectPoolInterface>())
    {
        UE_LOG(LogTemp, Warning, TEXT("UObjectPoolSubsystem::ReturnObjectToPool - Invalid actor or actor does not implement interface"));
        return;
    }

    if (FObjectPool* PoolObject = ObjectPoolMap.Find(ClassPool))
    {
        // Find an available position in the pool grid
        int32 PoolIndex = PoolObject->UsablePoolingObjects.Num();
        int32 GridX = PoolIndex % 10; // 10 objects per row
        int32 GridY = PoolIndex / 10; // Row number
        FVector PoolPosition = PoolVisualizationLocation + FVector(
            GridX * PoolGridSpacing,
            GridY * PoolGridSpacing,
            0.0f
        );

        // Create deactivation data
        FObjectPoolDeactivationData DeactivationData;
        DeactivationData.DeactivationLocation = PoolPosition;
        DeactivationData.DeactivationReason = FName("ReturnedToPool");
        DeactivationData.bShouldResetTransform = true;

        // Call deactivate on the actor
        if (IObjectPoolInterface* PoolInterface = Cast<IObjectPoolInterface>(ActorToReturn.GetObject()))
        {
            PoolInterface->Deactivate(DeactivationData);
        }

        // Ensure actor is visible in pool
        if (AActor* Actor = Cast<AActor>(ActorToReturn.GetObject()))
        {
            Actor->SetActorHiddenInGame(false);
            Actor->SetActorEnableCollision(false);
        }

        PoolObject->ActivePoolingObjects.RemoveSingleSwap(ActorToReturn);
        PoolObject->UsablePoolingObjects.AddUnique(ActorToReturn);

        UpdateStats();
    }
}

#pragma endregion

#pragma region STATS UPDATE

void UObjectPoolSubsystem::UpdateStats()
{
    int32 TotalPools = ObjectPoolMap.Num();
    int32 ActiveCount = 0;
    int32 FreeCount = 0;

    for (const auto& Pair : ObjectPoolMap)
    {
        ActiveCount += Pair.Value.ActivePoolingObjects.Num();
        FreeCount   += Pair.Value.UsablePoolingObjects.Num();
    }

    SET_DWORD_STAT(STAT_ObjectPool_TotalPools, TotalPools);
    SET_DWORD_STAT(STAT_ObjectPool_ActiveObjects, ActiveCount);
    SET_DWORD_STAT(STAT_ObjectPool_FreeObjects, FreeCount);
}

TArray<FPoolStatistics> UObjectPoolSubsystem::GetPoolStatistics() const
{
    TArray<FPoolStatistics> Statistics;

    for (const auto& Pair : ObjectPoolMap)
    {
        FPoolStatistics Stat;
        
        // Get class name
        if (Pair.Key && Pair.Key.Get())
        {
            Stat.ClassName = Pair.Key->GetName();
        }
        else
        {
            Stat.ClassName = TEXT("Unknown");
        }

        // Get counts
        Stat.ActiveObjects = Pair.Value.ActivePoolingObjects.Num();
        Stat.PullableObjects = Pair.Value.UsablePoolingObjects.Num();
        Stat.TotalObjects = Stat.ActiveObjects + Stat.PullableObjects;

        Statistics.Add(Stat);
    }

    return Statistics;
}

#pragma endregion 
