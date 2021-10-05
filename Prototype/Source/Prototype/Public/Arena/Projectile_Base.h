// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Damage.h"
#include "Projectile_Base.generated.h"


USTRUCT(BlueprintType)
struct FWeaponAttackTarget
{
    GENERATED_BODY()

public:

    UPROPERTY(BlueprintReadWrite, Category = "Unknown|Arena")
    AActor* mpTargetActor;

    UPROPERTY(BlueprintReadWrite, Category = "Unknown|Arena")
    FVector mTargetPosition;

    UPROPERTY(BlueprintReadWrite, Category = "Unknown|Arena")
    bool mIsMissing;
};


UCLASS(Abstract)
class PROTOTYPE_API AProjectile_Base : public AActor
{
    GENERATED_BODY()

public:

    DECLARE_DELEGATE_OneParam(FEventHit, AActor*);
    FEventHit mEventHit;

public:
    
    AProjectile_Base();

    UFUNCTION(BlueprintCallable, Category = "Unknown|Arena")
    void Launch(const FVector& from,
                const FVector& to,
                AActor* pDamageDealer,
                int32 executionPriority);

    UFUNCTION(BlueprintCallable, Category = "Unknown|Arena")
    void LaunchWithTargetActor(const FVector& from,
                               AActor* pTargetActor,
                               AActor* pDamageDealer,
                               int32 executionPriority);

    UFUNCTION(BlueprintCallable, Category = "Unknown|Arena")
    void LaunchWithWeaponAttackTarget(const FVector& from,
                                      const FWeaponAttackTarget& attackTarget,
                                      AActor* pDamageDealer,
                                      int32 executionPriority);

protected:

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unknown|Arena")
    FVector mGroundAcceleration;

    //  Defines XY speed of the projectile. Used to compute flight time.
    UPROPERTY(EditAnywhere, Category = "Unknown|Arena")
    float mSpeedXY = 1.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Unknown|Arena")
    FVector mVelocity;

    UPROPERTY(BlueprintReadWrite, Category = "Unknown|Arena")
    float mFlightTimer = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Unknown|Arena")
    FWeaponAttackTarget mAttackTarget;

    UPROPERTY(BlueprintReadOnly, Category = "Unknown|Arena")
    FVector mTargetPosition;

    UPROPERTY(BlueprintReadOnly, Category = "Unknown|Arena")
    AActor* mpTargetActor = nullptr;

    UPROPERTY(BlueprintReadOnly, Category = "Unknown|Arena")
    AActor* mpDamageDealer = nullptr;

    /*
        Can be used for setting the priority of delivered effects.
    */
    UPROPERTY(BlueprintReadOnly, Category = "Unknown|Arena")
    int32 mExecutionPriority = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Unknown|Arena")
    bool mIsLaunched = false;

    bool mIsMakingTurn = false;

    bool mIsMissing = false;

protected:
    // Called when the game starts or when spawned
    virtual void BeginPlay() override;

    // Called every frame
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintImplementableEvent, Category = "Unknown|Arena")
    void OnHit(float& destroyDelay);

    UFUNCTION(BlueprintImplementableEvent, Category = "Unknown|Arena")
    void OnMiss(float& destroyDelay);

    void DestroyProjectile();

    void StartMakingTurn();

    void StopMakingTurn();

    virtual void EndPlay(const EEndPlayReason::Type endPlayReason) override;
};
