// Fill out your copyright notice in the Description page of Project Settings.

#include "Game/Lesson12.09/ObjectPoolSubsystem.h"
#include "Game/Lesson12.09/ObjectPoolSettings.h"

#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Stats/Stats.h"
#include "UObject/UObjectGlobals.h"
#include "TimerManager.h"
#include "Game/Utility/Gauntlet_DebugHelper.h"

#pragma region STATS

double LastStatsUpdateTime = 0.0;

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
            }
        }
        return;
    }

    // Create new pool
    FObjectPool ObjectPoolToCreate;
    ObjectPoolToCreate.BaseSize = InitialSize;

    SpawnAndPlaceInPool(ClassPool, InitialSize, ObjectPoolToCreate);

    ObjectPoolMap.Add(ClassPool, ObjectPoolToCreate);

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

        TScriptInterface<IObjectPoolInterface> ActorToReturn;

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
            PoolObject->ActivePoolingObjects.Add(ActorToReturn);
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
        // SAFETY: prevent double return
        if (!PoolObject->ActivePoolingObjects.Contains(ActorToReturn))
        {
            return;
        }
        
        const UObjectPoolSettings* Settings = GetPoolSettings();
        FVector PoolPosition = FVector::ZeroVector;

#if WITH_EDITORONLY_DATA
        if (Settings)
        {
            // Find an available position in the pool grid
            int32 PoolIndex = PoolObject->UsablePoolingObjects.Num();
            int32 GridX = PoolIndex % 10; // 10 objects per row
            int32 GridY = PoolIndex / 10; // Row number
            PoolPosition = Settings->PoolVisualizationLocation + FVector(
                GridX * Settings->PoolGridSpacing,
                GridY * Settings->PoolGridSpacing,
                0.0f
            );
        }
#endif

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
        PoolObject->UsablePoolingObjects.Add(ActorToReturn);

        MaybeShrinkPool(ClassPool, *PoolObject);
    }
}

#pragma endregion

#pragma region STATS UPDATE

TArray<FPoolStatistics> UObjectPoolSubsystem::GetPoolStatistics() const
{
    TArray<FPoolStatistics> Statistics;
    Statistics.Reserve(ObjectPoolMap.Num());

    int32 Index = 0;
    for (const auto& Pair : ObjectPoolMap)
    {
        FPoolStatistics Stat;
        
        // Set pool index (0-based)
        Stat.PoolIndex = Index;
        
        // Get object name
        if (Pair.Key && Pair.Key.Get())
        {
            Stat.ObjectName = Pair.Key->GetName();
        }
        else
        {
            Stat.ObjectName = TEXT("Unknown");
        }

        // Get counts: InPool = available in pool, OutPool = active in scene
        Stat.InPool = Pair.Value.UsablePoolingObjects.Num();
        Stat.OutPool = Pair.Value.ActivePoolingObjects.Num();
        Stat.Total = Stat.InPool + Stat.OutPool;

        Statistics.Add(Stat);
        ++Index;
    }

    return Statistics;
}

TArray<FString> UObjectPoolSubsystem::GetPoolStatisticsAsStrings() const
{
    TArray<FString> OutputStrings;
    
    if (ObjectPoolMap.Num() == 0)
    {
        OutputStrings.Add(TEXT("No pools available"));
        return OutputStrings;
    }

    // Header
    OutputStrings.Add(TEXT("======================================== Object Pool Statistics ========================================"));
    OutputStrings.Add(FString::Printf(TEXT("%-6s | %-32s | %-8s | %-8s | %-8s"), 
        TEXT("Index"), TEXT("Object Name"), TEXT("In Pool"), TEXT("Out Pool"), TEXT("Total")));
    OutputStrings.Add(TEXT("--------------------------------------------------------------------------------------------------------"));
    
    int32 ObjectsOutPool = 0;
    int32 ObjectsInPool = 0;
    
    // Print each pool and accumulate totals
    int32 Index = 0;
    for (const auto& Pair : ObjectPoolMap)
    {
        FString ObjectName = TEXT("Unknown");
        if (Pair.Key && Pair.Key.Get())
        {
            ObjectName = Pair.Key->GetName();
        }

        int32 InPool = Pair.Value.UsablePoolingObjects.Num();
        int32 OutPool = Pair.Value.ActivePoolingObjects.Num();
        int32 Total = InPool + OutPool;

        // Print row
        OutputStrings.Add(FString::Printf(TEXT("%-6d | %-32s | %-8d | %-8d | %-8d"), 
            Index, *ObjectName, InPool, OutPool, Total));
        
        ObjectsOutPool += OutPool;
        ObjectsInPool += InPool;
        ++Index;
    }

    // Print footer with totals
    OutputStrings.Add(TEXT("--------------------------------------------------------------------------------------------------------"));
    OutputStrings.Add(FString::Printf(TEXT("%-6s | %-32s | %-8d | %-8d | %-8d"), 
        TEXT("TOTAL"), TEXT(""), ObjectsInPool, ObjectsOutPool, ObjectsInPool + ObjectsOutPool));
    OutputStrings.Add(TEXT("========================================================================================================"));
    
    return OutputStrings;
}

#pragma endregion 

#pragma region HELPERS

const UObjectPoolSettings* UObjectPoolSubsystem::GetPoolSettings() const
{
    return GetDefault<UObjectPoolSettings>();
}

void UObjectPoolSubsystem::RecordUsageAndMaybeGrow(
    TSubclassOf<AActor> ClassPool,
    FObjectPool& Pool)
{
    if (!GetWorld())
    {
        return;
    }

    const UObjectPoolSettings* Settings = GetPoolSettings();
    if (!Settings)
    {
        return;
    }

    const double Now = GetWorld()->GetTimeSeconds();

    // Record this request
    Pool.RecentRequestTimes.Add(Now);

    // Clean old requests outside the window
    const double WindowStart = Now - Settings->ConsumptionRateWindowSeconds;
    Pool.RecentRequestTimes.RemoveAll([WindowStart](double Time) { return Time < WindowStart; });

    // Calculate consumption rate (objects per second)
    const int32 RequestsInWindow = Pool.RecentRequestTimes.Num();
    if (RequestsInWindow == 0 || Settings->ConsumptionRateWindowSeconds <= 0.0f)
    {
        return;
    }

    const float ConsumptionRate = static_cast<float>(RequestsInWindow) / Settings->ConsumptionRateWindowSeconds;
    
    const int32 FreeCount = Pool.UsablePoolingObjects.Num();
    if (FreeCount <= 0 || ConsumptionRate <= 0.0f)
    {
        return;
    }

    // Calculate time to exhaustion: FreeCount / ConsumptionRate
    const float TimeToExhaustion = static_cast<float>(FreeCount) / ConsumptionRate;

    // GROW if time to exhaustion is below minimum threshold
    if (TimeToExhaustion < Settings->MinExhaustionTimeSeconds)
    {
        // Calculate desired free count to maintain minimum exhaustion time
        const float DesiredFreeCount = ConsumptionRate * Settings->MinExhaustionTimeSeconds;
        const int32 ToSpawn = FMath::Max(
            Settings->MinObjectsToAdd,
            FMath::CeilToInt(DesiredFreeCount - FreeCount)
        );

        if (ToSpawn > 0)
        {
            SpawnAndPlaceInPool(ClassPool, ToSpawn, Pool);
        }
    }
}

void UObjectPoolSubsystem::SpawnAndPlaceInPool(const TSubclassOf<AActor> ClassPool, int32 Count, FObjectPool& Pool)
{
    if (!GetWorld() || Count <= 0)
    {
        return;
    }

    const UObjectPoolSettings* Settings = GetPoolSettings();
    if (!Settings)
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
#if WITH_EDITORONLY_DATA
            int32 GlobalIndex = StartIndex + i;
            int32 GridX = GlobalIndex % 10; // 10 objects per row
            int32 GridY = GlobalIndex / 10; // Row number
            FVector PoolPosition = Settings->PoolVisualizationLocation + FVector(
                GridX * Settings->PoolGridSpacing,
                GridY * Settings->PoolGridSpacing,
                0.0f
            );

            ActorRef->SetActorLocation(PoolPosition);
#endif
            ActorRef->SetActorHiddenInGame(false);
            ActorRef->SetActorEnableCollision(false);

            Pool.UsablePoolingObjects.Add(ActorRef);
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

void UObjectPoolSubsystem::MaybeShrinkPool(
    TSubclassOf<AActor> ClassPool,
    FObjectPool& Pool)
{
    if (!GetWorld())
    {
        return;
    }

    const UObjectPoolSettings* Settings = GetPoolSettings();
    if (!Settings)
    {
        return;
    }

    const int32 FreeCount = Pool.UsablePoolingObjects.Num();
    const int32 ActiveCount = Pool.ActivePoolingObjects.Num();
    const int32 TotalCount = FreeCount + ActiveCount;

    // Never shrink below base size
    if (TotalCount <= Pool.BaseSize)
    {
        return;
    }

    const double Now = GetWorld()->GetTimeSeconds();
    
    // Clean old requests to calculate current consumption rate
    const double WindowStart = Now - Settings->ConsumptionRateWindowSeconds;
    Pool.RecentRequestTimes.RemoveAll([WindowStart](double Time) { return Time < WindowStart; });

    const int32 RequestsInWindow = Pool.RecentRequestTimes.Num();
    if (RequestsInWindow == 0 || Settings->ConsumptionRateWindowSeconds <= 0.0f)
    {
        // No recent activity, shrink towards base size
        const int32 Excess = TotalCount - Pool.BaseSize;
        const int32 ToDestroy = FMath::Min(Excess, FreeCount);

        for (int32 i = 0; i < ToDestroy; ++i)
        {
            const int32 Index = Pool.UsablePoolingObjects.Num() - 1;
            if (Index < 0) break;

            TScriptInterface<IObjectPoolInterface> Obj = Pool.UsablePoolingObjects[Index];
            Pool.UsablePoolingObjects.RemoveAt(Index);

            if (AActor* Actor = Cast<AActor>(Obj.GetObject()))
            {
                Actor->Destroy();
            }
        }
        return;
    }

    // Calculate current consumption rate
    const float ConsumptionRate = static_cast<float>(RequestsInWindow) / Settings->ConsumptionRateWindowSeconds;
    
    if (FreeCount <= 0 || ConsumptionRate <= 0.0f)
    {
        return;
    }

    // Calculate time to exhaustion
    const float TimeToExhaustion = static_cast<float>(FreeCount) / ConsumptionRate;

    // SHRINK if time to exhaustion is above maximum threshold
    if (TimeToExhaustion > Settings->MaxExhaustionTimeSeconds)
    {
        // Calculate desired free count based on target exhaustion time
        const float DesiredFreeCount = ConsumptionRate * Settings->TargetExhaustionTimeSeconds;
        
        // Ensure we never go below base size
        const int32 MinFreeCount = FMath::Max(0, Pool.BaseSize - ActiveCount);
        const int32 TargetFreeCount = FMath::Max(
            MinFreeCount,
            FMath::CeilToInt(DesiredFreeCount)
        );

        const int32 Excess = FreeCount - TargetFreeCount;
        const int32 ToDestroy = FMath::Clamp(Excess, 0, FreeCount);

        for (int32 i = 0; i < ToDestroy; ++i)
        {
            const int32 Index = Pool.UsablePoolingObjects.Num() - 1;
            if (Index < 0) break;

            TScriptInterface<IObjectPoolInterface> Obj = Pool.UsablePoolingObjects[Index];
            Pool.UsablePoolingObjects.RemoveAt(Index);

            if (AActor* Actor = Cast<AActor>(Obj.GetObject()))
            {
                Actor->Destroy();
            }
        }
    }
}

#pragma endregion
