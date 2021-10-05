// Fill out your copyright notice in the Description page of Project Settings.

#include "Arena/Projectile_Base.h"
#include "Arena/ArenaUtilities.h"
#include "Arena/Damageable.h"
#include "Arena/TurnsManager.h"
#include "TimerManager.h"


// Sets default values
AProjectile_Base::AProjectile_Base()
    : mGroundAcceleration(0.0f, 0.0f, -980.0f)
    , mVelocity(0.0f, 0.0f, 0.0f)
    , mFlightTimer(0.0f)
    , mpTargetActor(nullptr)
    , mpDamageDealer(nullptr)
    , mExecutionPriority(0)
    , mIsLaunched(false)
    , mIsMakingTurn(false)
    , mIsMissing(false)
{
    // Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
    PrimaryActorTick.bCanEverTick = true;
}

void AProjectile_Base::Launch(const FVector& from,
                              const FVector& to,
                              AActor* pDamageDealer,
                              int32 executionPriority)
{
    SetActorLocation(from);
    float flightDuration = FVector::DistXY(from, to) / mSpeedXY;
    mVelocity = (to - from) / flightDuration - 0.5f * flightDuration * mGroundAcceleration;
    mFlightTimer = flightDuration;
    mIsLaunched = true;
    mpDamageDealer = pDamageDealer;
    mExecutionPriority = executionPriority;
    mTargetPosition = to;
    StartMakingTurn();
}

void AProjectile_Base::LaunchWithTargetActor(const FVector& from,
                                             AActor* pTargetActor,
                                             AActor* pDamageDealer,
                                             int32 executionPriority)
{
    //	~NF~ ~TODO~ Of course the projectile should be launched in a different way of launch with target tile. It should follow.
    check(pTargetActor != nullptr && "AProjectile_Base::LaunchWithTargetActor(): Target actor is invalid!");
    mpTargetActor = pTargetActor;
    FVector to = mpTargetActor->GetActorLocation() + FVector(0.0f, 0.0f, 100.0f);
    Launch(from, to, pDamageDealer, executionPriority);
}

void AProjectile_Base::LaunchWithWeaponAttackTarget(const FVector& from,
                                                    const FWeaponAttackTarget& attackTarget,
                                                    AActor* pDamageDealer,
                                                    int32 executionPriority)
{
    mIsMissing = attackTarget.mIsMissing;
    if (attackTarget.mpTargetActor != nullptr)
    {
        LaunchWithTargetActor(from,
                              attackTarget.mpTargetActor,
                              pDamageDealer,
                              executionPriority);
    }
    else
    {
        Launch(from,
               attackTarget.mTargetPosition,
               pDamageDealer,
               executionPriority);
    }
}

void AProjectile_Base::BeginPlay()
{
    Super::BeginPlay();
}

void AProjectile_Base::Tick(float deltaTime)
{
    Super::Tick(deltaTime);

    if (mIsLaunched == false)
    {
        return;
    }

    mFlightTimer -= deltaTime;
    if (mFlightTimer <= 0.0f)
    {
        mIsLaunched = false;
        float destroyDelay = 0;
        if (mIsMissing)
        {
            OnMiss(destroyDelay);
        }
        else
        {
            OnHit(destroyDelay);
            mEventHit.ExecuteIfBound(mpTargetActor);
        }
        if (destroyDelay <= 0.0f)
        {
            DestroyProjectile();
        }
        else
        {
            FTimerHandle unusedHandle;
            GetWorldTimerManager().SetTimer(unusedHandle,
                                            this,
                                            &AProjectile_Base::DestroyProjectile,
                                            destroyDelay);
        }
        return;
    }

    FVector curPosition = GetActorLocation();
    FVector newPosition;
    FRotator newRotation;
    if (IsValid(mpTargetActor))
    {
        mTargetPosition = mpTargetActor->GetActorLocation();
    }
    newPosition = FMath::Lerp(curPosition, mTargetPosition, deltaTime / mFlightTimer);
    newPosition.Z = curPosition.Z + mVelocity.Z * deltaTime + 0.5f * mGroundAcceleration.Z * deltaTime * deltaTime;
    mVelocity += deltaTime * mGroundAcceleration;

    SetActorLocationAndRotation(newPosition, newRotation);
}

void AProjectile_Base::DestroyProjectile()
{
    StopMakingTurn();

    //	~R~ ~TODO~ When using projectiles from pools, they shouldn't be destroyed.
    Destroy();
}

void AProjectile_Base::StartMakingTurn()
{
    mIsMakingTurn = true;
    UArenaUtilities::GetTurnsManager(this)->QueryTurnExecution(this);
}

void AProjectile_Base::StopMakingTurn()
{
    if (mIsMakingTurn && UArenaUtilities::AreSystemsSetup(this))
    {
        UArenaUtilities::GetTurnsManager(this)->ReleaseTurnExecution(this);
        mIsMakingTurn = false;
    }
}

void AProjectile_Base::EndPlay(const EEndPlayReason::Type endPlayReason)
{
    StopMakingTurn();

    Super::EndPlay(endPlayReason);
}
