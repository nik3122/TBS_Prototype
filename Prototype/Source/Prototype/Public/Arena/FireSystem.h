// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PassMap.h"
#include "Containers/Set.h"
#include "ArenaState.h"
#include "Version.h"
#include "Damage.h"
#include "StatusEffectsManager.h"
#include "FireSystem.generated.h"


class APrototypeGameModeBase;
class UTiledMovement;
class UDamageable;


struct State_FireSystem
    : State_Base
{
    DECLARE_CLASS_VERSION(1, 0);
    int32* mpTilesToProcess;                //  Indices of tiles.
    int32 mTilesToProcessCount;
    int32* mpTilesOnFire;                   //  Indices of tiles.
    int32 mTilesOnFireCount;
};


USTRUCT(BlueprintType)
struct PROTOTYPE_API FTileFireData
{
    GENERATED_BODY()


public:
    bool mIsOnFire = false;
};


UCLASS()
class PROTOTYPE_API AFireSystem
    : public AActor
    , public ArenaStateObject_Base<State_FireSystem>
{
    CHECK_CLASS_VERSION_EQUAL(State_FireSystem, 1, 0);

    GENERATED_BODY()
    
public:	

    AFireSystem(const FObjectInitializer& objectInitializer = FObjectInitializer::Get());

    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnFireStarted, FIntPoint, Tile);
    UPARAM(BlueprintReadWrite, Category = "Arena")
    FOnFireStarted mOnFireStarted;

public:

    void SetupSystem(ATurnsManager* pTurnsManager, APassMap* pPassMap);

    //  ~!~HACK~UGLY~ Its used by movement actions on PreExecutionApply to trigger traps.
    //  It is crucial for actions to call this in order of execution to work narratively.
    //  ~?~!~ Alternate solution is to make locomotion "core" mechanics for AAction_Base
    //  and automatically resolve all movement-related issues underhood of the systems.
    void ProcessMovement(const TArray<FIntPoint>& path, ACreatureBase* pMovingCreature);

    //  ~UGLY~ Called to process all owners of the registered tiled movements
    void ProcessAllMovements();

    void UpdateFireDistribution();

    UFUNCTION(BlueprintPure, Category = "Arena")
    bool IsTileIgnitable(const FIntPoint& tile);

    UFUNCTION(BlueprintCallable, Category = "Arena")
    void SetTileOnFire(const FIntPoint& tile);

    UFUNCTION(BlueprintPure, Category = "Arena")
    bool IsTileOnFire(const FIntPoint& tile) const;

    UFUNCTION(BlueprintCallable, Category = "Arena")
    void ExtinguishFireOnTile(const FIntPoint& tile);

protected:

    UPROPERTY(BlueprintReadOnly, Category = "Arena")
    APassMap* mpPassMap;

    UPROPERTY(BlueprintReadOnly, Category = "Arena")
    FIntPoint mMapSize;

    UPROPERTY(BlueprintReadOnly, Category = "Arena")
    TArray<FTileFireData> mTilesFireData;
    
    UPROPERTY()
    TArray<FIntPoint> mTilesToProcess;

    UPROPERTY()
    TArray<FIntPoint> mTilesOnFire;

    //  Damage to deal to actor trapped into fire.
    UPROPERTY(EditDefaultsOnly, Category = "Arena")
    FDamageParameters mDamageToDealOnTrapped;

    //  Status effect (burning) to activate for actor trapped into fire.
    UPROPERTY(EditDefaultsOnly, Category = "Arena")
    FStatusEffectTask mStatusEffectTaskOnTrapped;

    //  ~UGLY~ Used to store already processed movements passed into ProcessMovement().
    TSet<UTiledMovement*> mProcessedMovements;

protected:

    void BeginPlay() override;

    UFUNCTION()
    void OnFinalizeTurn();

    UFUNCTION()
    void OnPassMapMovementRegistered(UTiledMovement* pPassMapMovement);

    UFUNCTION()
    void OnTileReach(UTiledMovement* pTiledMovement, const FIntPoint& tile);

    UFUNCTION(BlueprintPure, Category = "Arena")
    APassMap* GetPassMap() const;

    UFUNCTION(BlueprintCallable, Category = "Arena")
    void SetTileOnFire_Inner(const FIntPoint& tile,
                             FTileFireData& tileData);

    UFUNCTION(BlueprintImplementableEvent, Category = "Arena")
    void NewTileOnFire(const FIntPoint& newTile);

    UFUNCTION(BlueprintImplementableEvent, Category = "Arena")
    void UpdateTileOnFire(const FIntPoint& tile);

    UFUNCTION(BlueprintImplementableEvent, Category = "Arena")
    void OnActorTrappedInFire(const FIntPoint& tile,
                              AActor* pActor);

    UFUNCTION(BlueprintPure, Category = "Arena")
    FTileFireData& GetTileFireData(const FIntPoint& tile);
        
    const FTileFireData& GetTileFireData(const FIntPoint& tile) const;

    void StoreState_Inner(ArenaStateContext& context, State_FireSystem& state) const override;

    bool RestoreState_Inner(const State_FireSystem& state) override;

    void AffectTrappedInFire(int32 trappedTimes,
                             UDamageable* pDamageable,
                             UStatusEffectsManager* pStatusEffectsManager);
};
