// Fill out your copyright notice in the Description page of Project Settings.

#include "Game/Lesson12.09/ObjectPoolSubsystem.h"

#include "Engine/World.h"
#include "Stats/Stats.h"
#include "UObject/UObjectGlobals.h"
#include "TimerManager.h"
#include "Game/Utility/Gauntlet_DebugHelper.h"

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
        UGauntlet_DebugHelper::ShowWarning(TEXT("UObjectPoolSubsystem::AddPool - Invalid class provided"));
        return;
    }

    // Check if class implements the interface
    if (!DoesClassImplementInterface(ClassPool))
    {
        UGauntlet_DebugHelper::ShowWarning(FString::Printf(TEXT("UObjectPoolSubsystem::AddPool - Class %s does not implement IObjectPoolInterface"), *GetNameSafe(ClassPool.Get())));
        return;
    }

    // Check if pool already exists for this class
    if (ObjectPoolMap.Contains(ClassPool))
    {
        // Pool already exists, use the existing one
        // Optionally prewarm if the existing pool is smaller than requested
        if (FObjectPool* ExistingPool = ObjectPoolMap.Find(ClassPool))
        {
            const int32 CurrentTotal = ExistingPool->UsablePoolingObjects.Num() + ExistingPool->ActivePoolingObjects.Num();
            if (CurrentTotal < InitialSize)
            {
                // Prewarm to reach the requested initial size
                const int32 AdditionalNeeded = InitialSize - CurrentTotal;
                SpawnAndPlaceInPool(ClassPool, AdditionalNeeded, *ExistingPool);
                ExistingPool->BaseSize = FMath::Max(ExistingPool->BaseSize, InitialSize);
                UpdateStats();
            }
        }
        return;
    }

    // Create new pool
    FObjectPool ObjectPoolToCreate;
    ObjectPoolToCreate.BaseSize = InitialSize;
    ObjectPoolToCreate.LastRequestTime = GetWorld()->GetTimeSeconds();
    ObjectPoolToCreate.RecentRequestTimes.Empty();

    SpawnAndPlaceInPool(ClassPool, InitialSize, ObjectPoolToCreate);

    ObjectPoolMap.Add(ClassPool, ObjectPoolToCreate);

    UpdateStats();
}

void UObjectPoolSubsystem::PrewarmPool(const TSubclassOf<AActor> ClassPool, int32 AdditionalSize)
{
    if (!GetWorld() || AdditionalSize <= 0)
    {
        return;
    }

    if (FObjectPool* PoolObject = ObjectPoolMap.Find(ClassPool))
    {
        SpawnAndPlaceInPool(ClassPool, AdditionalSize, *PoolObject);
        UpdateStats();
    }
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
        UGauntlet_DebugHelper::ShowWarning(TEXT("UObjectPoolSubsystem::GetObjectFromPool - Invalid class provided"));
        return nullptr;
    }

    // Check if class implements the interface
    if (!DoesClassImplementInterface(ClassPool))
    {
        UGauntlet_DebugHelper::ShowWarning(FString::Printf(TEXT("UObjectPoolSubsystem::GetObjectFromPool - Class %s does not implement IObjectPoolInterface"), *GetNameSafe(ClassPool.Get())));
        return nullptr;
    }

    if (FObjectPool* PoolObject = ObjectPoolMap.Find(ClassPool))
    {
        RecordUsageAndMaybeGrow(ClassPool, *PoolObject);

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
                UGauntlet_DebugHelper::ShowWarning(TEXT("UObjectPoolSubsystem::GetObjectFromPool - Spawned actor does not implement interface"));
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
        UGauntlet_DebugHelper::ShowWarning(TEXT("UObjectPoolSubsystem::ReturnObjectToPool - Invalid class provided"));
        return;
    }

    // Check if actor is valid and implements interface
    if (!ActorToReturn.GetObject() || !ActorToReturn.GetObject()->Implements<UObjectPoolInterface>())
    {
        UGauntlet_DebugHelper::ShowWarning(TEXT("UObjectPoolSubsystem::ReturnObjectToPool - Invalid actor or actor does not implement interface"));
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
            /*
            FString Msg = FString::Printf(
                TEXT("UObjectPoolSubsystem::ReturnObjectToPool - %s deactivated and returned to pool"),
                *GetNameSafe(ActorToReturn.GetObject())
                );
            UGauntlet_DebugHelper::ShowInfo(Msg);
            */
        }

        // Ensure actor is visible in pool
        if (AActor* Actor = Cast<AActor>(ActorToReturn.GetObject()))
        {
            Actor->SetActorHiddenInGame(false);
            Actor->SetActorEnableCollision(false);
        }

        PoolObject->ActivePoolingObjects.RemoveSingleSwap(ActorToReturn);
        PoolObject->UsablePoolingObjects.AddUnique(ActorToReturn);

        MaybeShrinkPool(ClassPool, *PoolObject);

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

#pragma region HELPERS

void UObjectPoolSubsystem::SpawnAndPlaceInPool(const TSubclassOf<AActor> ClassPool, int32 Count, FObjectPool& Pool)
{
    if (!GetWorld() || Count <= 0)
    {
        return;
    }

    FActorSpawnParameters SpawnParameters;
    SpawnParameters.SpawnCollisionHandlingOverride =
        ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    int32 StartIndex = Pool.UsablePoolingObjects.Num() + Pool.ActivePoolingObjects.Num();

    for (int32 i = 0; i < Count; ++i)
    {
        AActor* ActorRef = GetWorld()->SpawnActor<AActor>(ClassPool, SpawnParameters);
        
        if (ActorRef && ActorRef->Implements<UObjectPoolInterface>())
        {
            int32 GlobalIndex = StartIndex + i;
            int32 GridX = GlobalIndex % 10; // 10 objects per row
            int32 GridY = GlobalIndex / 10; // Row number
            FVector PoolPosition = PoolVisualizationLocation + FVector(
                GridX * PoolGridSpacing,
                GridY * PoolGridSpacing,
                0.0f
            );

            ActorRef->SetActorLocation(PoolPosition);
            ActorRef->SetActorHiddenInGame(false);
            ActorRef->SetActorEnableCollision(false);

            Pool.UsablePoolingObjects.AddUnique(ActorRef);
        }
        else
        {
            UGauntlet_DebugHelper::ShowWarning(TEXT("UObjectPoolSubsystem::SpawnAndPlaceInPool - Failed to spawn or actor does not implement interface"));
            if (ActorRef)
            {
                ActorRef->Destroy();
            }
        }
    }
}

void UObjectPoolSubsystem::RecordUsageAndMaybeGrow(TSubclassOf<AActor> ClassPool, FObjectPool& Pool)
{
    if (!GetWorld())
    {
        return;
    }

    const double Now = GetWorld()->GetTimeSeconds();
    Pool.LastRequestTime = Now;
    Pool.RecentRequestTimes.Add(Now);

    // Clean old entries
    const double WindowStart = Now - AutoScaleWindowSeconds;
    Pool.RecentRequestTimes.RemoveAll([WindowStart](double T) { return T < WindowStart; });

    const int32 RequestsInWindow = Pool.RecentRequestTimes.Num();
    if (RequestsInWindow == 0 || AutoScaleWindowSeconds <= 0.01f)
    {
        return;
    }

    const float Rate = static_cast<float>(RequestsInWindow) / AutoScaleWindowSeconds; // requests per second
    const int32 FreeCount = Pool.UsablePoolingObjects.Num();

    if (Rate <= 0.0f)
    {
        return;
    }

    const float ProjectedExhaustion = (Rate > 0.0f) ? (FreeCount / Rate) : FLT_MAX;

    if (ProjectedExhaustion < AutoScaleLookaheadSeconds)
    {
        // Estimate how many we need to cover the lookahead plus a small buffer
        const int32 NeededToCover = FMath::CeilToInt(Rate * AutoScaleLookaheadSeconds) - FreeCount;
        const int32 SpawnCount = FMath::Max(NeededToCover, AutoScaleMinAdd);
        SpawnAndPlaceInPool(ClassPool, SpawnCount, Pool);
    }
}

void UObjectPoolSubsystem::MaybeShrinkPool(TSubclassOf<AActor> ClassPool, FObjectPool& Pool)
{
    if (!GetWorld())
    {
        return;
    }

    const double Now = GetWorld()->GetTimeSeconds();
    const double TimeSinceLastRequest = Now - Pool.LastRequestTime;

    const int32 TotalCount = Pool.UsablePoolingObjects.Num() + Pool.ActivePoolingObjects.Num();
    if (TotalCount <= Pool.BaseSize)
    {
        return;
    }

    if (TimeSinceLastRequest < InactivityShrinkSeconds)
    {
        return;
    }

    // Destroy surplus from the usable list only
    const int32 Surplus = TotalCount - Pool.BaseSize;
    int32 ToRemove = FMath::Min(Surplus, Pool.UsablePoolingObjects.Num());

    for (int32 i = 0; i < ToRemove; ++i)
    {
        const int32 Index = Pool.UsablePoolingObjects.Num() - 1;
        TScriptInterface<IObjectPoolInterface> Obj = Pool.UsablePoolingObjects[Index];
        Pool.UsablePoolingObjects.RemoveAt(Index);

        if (AActor* Actor = Cast<AActor>(Obj.GetObject()))
        {
            Actor->Destroy();
        }
    }
}

#pragma endregion
