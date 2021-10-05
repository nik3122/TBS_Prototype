// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "Arena/OccupationSlot.h"
#include "Arena/ArenaState.h"
#include "Arena/StatusEffect.h"
#include "Version.h"
#include "TiledMovement.generated.h"


class UTiledMovement;
class UStatusEffectsManager;
class UDamageable;


struct State_TiledMovement
    : public State_Base
{
    DECLARE_CLASS_VERSION(1, 0);
    int32 mTileSpeed;
    FIntPoint mCurTile;
    bool mIsStunned;
};


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PROTOTYPE_API UTiledMovement
    : public UActorComponent
    , public ArenaStateObject_Base<State_TiledMovement>
{
    CHECK_CLASS_VERSION_EQUAL(State_TiledMovement, 1, 0);

    GENERATED_BODY()

public:

    DECLARE_DELEGATE_OneParam(FDelegateMovementStarted, UTiledMovement*);
    DECLARE_MULTICAST_DELEGATE_OneParam(FEventMovementStarted, UTiledMovement*);
    FEventMovementStarted mEventMovementStarted;

    DECLARE_DELEGATE_OneParam(FDelegateMovementStopped, UTiledMovement*);
    DECLARE_MULTICAST_DELEGATE_OneParam(FEventMovementStopped, UTiledMovement*);
    FEventMovementStopped mEventMovementStopped;

    DECLARE_DYNAMIC_MULTICAST_DELEGATE(FCallbackOnStart);
    UPROPERTY(BlueprintAssignable, Category = "Unknown|TiledMovement")
    FCallbackOnStart mOnStart;

    DECLARE_DYNAMIC_MULTICAST_DELEGATE(FCallbackOnStop);
    UPROPERTY(BlueprintAssignable, Category = "Unknown|TiledMovement")
    FCallbackOnStop mOnStop;

    DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FEventTileReached, UTiledMovement*, pTiledMovement, const FIntPoint&, tile);
    UPROPERTY(BlueprintAssignable, Category = "Unknown|TiledMovement")
    FEventTileReached mEventTileReached;

    UFUNCTION(BlueprintPure, Category = "Unknown|TiledMovement")
    static float GetMovementDirectionFinalViewAngle();

public:
    
    UTiledMovement(const FObjectInitializer& objectInitializer = FObjectInitializer::Get());
    
    void InitializeComponent() override;

    UFUNCTION(BlueprintPure, Category = "Unknown|TiledMovement")
    APassMap* GetPassMap() const;

    void EnableOccupation(bool enableOccupation);

    bool IsOccupationEnabled() const;

    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    void SetLocation(const FIntPoint& tile);

    //	Sets a tile in which an actor is physically based as the current tile.
    void SyncTileByPos();

    //	Sets stunned status:
    UFUNCTION(BlueprintCallable, Category = "Unknown|TiledMovement")
    void SetStunned(bool isStunned);

    //	Returns stun status:
    UFUNCTION(BlueprintCallable, Category = "Unknown|TiledMovement")
    bool IsStunned() const;

    //	Moves an actor to the target tile, the path is determined automaticly.
    UFUNCTION(BlueprintCallable, Category = "Unknown|TiledMovement")
    bool MoveTo(const FIntPoint& targetTile,
                float finalViewAngle);

    //	Moves an actor to the target tile, during the specified time.
    UFUNCTION(BlueprintCallable, Category = "Unknown|TiledMovement")
    bool MoveToTimed(const FIntPoint& targetTile,
                     float timeToProcessMovement,
                     float finalViewAngle);

    //	Moves an actor by the specified path.
    UFUNCTION(BlueprintCallable, Category = "Unknown|TiledMovement")
    bool MovePath(const TArray<FIntPoint>& pathTiles,
                  float finalViewAngle);

    //	Moves an actor by the specified path during the specified time.
    UFUNCTION(BlueprintCallable, Category = "Unknown|TiledMovement")
    bool MovePathTimed(const TArray<FIntPoint>& pathTiles,
                       float timeToProcessMovement,
                       float finalViewAngle);

    //	Sets placement for actor. Returns true if new placement is set.
    UFUNCTION(BlueprintCallable, Category = "Unknown|TiledMovement")
    bool TrySetPlacement(const FIntPoint& placement);

    //	Gets tile which the actor currently occupies.
    UFUNCTION(BlueprintCallable, Category = "Unknown|TiledMovement")
    FIntPoint GetCurrentTile() const;

    //	Sets physical speed.
    UFUNCTION(BlueprintCallable, Category = "Unknown|TiledMovement")
    void SetPhysicalSpeed(float speed);

    //	Gets physical speed of an actor.
    UFUNCTION(BlueprintCallable, Category = "Unknown|TiledMovement")
    float GetPhysicalSpeed() const;

    //	Sets angular speed.
    UFUNCTION(BlueprintCallable, Category = "Unknown|TiledMovement")
    void SetAngularSpeed(float speed);
    
    //	Gets angular speed of an actor.
    UFUNCTION(BlueprintCallable, Category = "Unknown|TiledMovement")
    float GetAngularSpeed() const;

    //	Returns true if actor is currently moving.
    UFUNCTION(BlueprintCallable, Category = "Unknown|TiledMovement")
    bool IsMoving() const;

    UFUNCTION(BlueprintCallable, Category = "Unknown|TiledMovement")
    void SetTileSpeed(int32 tilesPerTurn);

    UFUNCTION(BlueprintPure, Category = "Unknown|TiledMovement")
    int32 GetTileSpeed() const;

    UFUNCTION(BlueprintCallable, Category = "Unknown|TiledMovement")
    void SetTileSpeedBonus(int32 speedBonus);

    UFUNCTION(BlueprintPure, Category = "Unknown|TiledMovement")
    int32 GetTileSpeedBonus() const;

    UFUNCTION(BlueprintCallable, Category = "Unknown|TiledMovement")
    void RotateTo(float viewAngle);

    UFUNCTION(BlueprintCallable, Category = "Unknown|TiledMovement")
    void AimAt(const FVector& aimTarget);

    UFUNCTION(BlueprintCallable, Category = "Unknown|TiledMovement")
    void AimAtTile(const FIntPoint& aimTargetTile);

    UFUNCTION(BlueprintCallable, Category = "Unknown|TiledMovement")
    void SetOrientation(float viewAngle);

    UFUNCTION(BlueprintPure, Category = "Unknown|TiledMovement")
    float GetOrientation() const;

    UFUNCTION(BlueprintPure, Category = "Unknown|TiledMovement")
    float GetCurrentPhysicalSpeed() const;

    //UFUNCTION(BlueprintPure, Category = "Unknown|TiledMovement")
    //int32 GetOccupationMask() const;

    UFUNCTION(BlueprintPure, Category = "Unknown|TiledMovement")
    EOccupationSlot GetOccupationSlot() const;

protected:
    //	Movement characteristics:

    // Nominal movement speed. When "timed" functions are used, this parameter is not counted.
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Unknown|TiledMovement")
    float mPhysicalSpeed;

    // Nominal angular speed. When "timed" functions are used, this parameter is not counted.
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Unknown|TiledMovement")
    float mAngularSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Unknown|TiledMovement")
    int32 mTileSpeed;

    UPROPERTY(EditAnywhere, Category = "Unknown|TiledMovement")
    bool mSyncPosByTileAtSpawn;

    //UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Unknown|TiledMovement", Meta = (Bitmask, BitmaskEnum = "EOccupationSlots"))
    //int32 mOccupationMask;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Unknown|TiledMovement")
    EOccupationSlot mOccupationSlot;

protected:

    void BeginPlay() override;

    void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

    UFUNCTION()
    void OnSystemsSetup();

protected:
    //	Movement data:
    UPROPERTY()
    FIntPoint mCurTile;

    UPROPERTY()
    TArray<FIntPoint> mPath;

    UPROPERTY()
    float mViewAngle;

    UPROPERTY()
    float mFinalViewAngle;

    UPROPERTY()
    float mTimeToMove;

    UPROPERTY()
    FVector mMovementDir;

    UPROPERTY()
    float mCurrentPhysicalSpeed;

    UPROPERTY()
    int32 mTileSpeedBonus;

    //	Current target rotation.
    UPROPERTY()
    FRotator mTargetRotation;

    //	Rotation which should be set after movement.
    UPROPERTY()
    FRotator mMovementFinalizationRotation;

    UPROPERTY()
    bool mMoving;

    UPROPERTY()
    bool mIsStunned;

    UPROPERTY()
    bool mIsOccupationEnabled;

    UPROPERTY()
    UStatusEffectsManager* mpOwnerStatusEffectsManager;

    UPROPERTY()
    UDamageable* mpOwnerDamageable = nullptr;

private:

    void OnMovementStop();

    void OnTileReach();

    void Move(float DeltaTime);

    void Rotate(float DeltaTime);

    void StoreState_Inner(ArenaStateContext& context, State_TiledMovement& state) const override;

    bool RestoreState_Inner(const State_TiledMovement& state) override;

};
