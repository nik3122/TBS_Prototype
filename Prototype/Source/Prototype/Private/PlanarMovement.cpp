
#include "PlanarMovement.h"


UPlanarMovement::UPlanarMovement(const FObjectInitializer& objInit)
    : Super(objInit)
{
    bConstrainToPlane = true;
    PlaneConstraintOrigin = FVector(0.0f, 0.0f, 0.0f);
    PlaneConstraintNormal = FVector(0.0f, 0.0f, 1.0f);
}

void UPlanarMovement::TickComponent(float deltaTime, ELevelTick tickType, FActorComponentTickFunction *pThisTickFunction)
{
    Super::TickComponent(deltaTime, tickType, pThisTickFunction);

    if (ShouldSkipUpdate(deltaTime)) return;

    FRotator rotation = GetOwner()->GetActorRotation() + mRotationSpeed * deltaTime;

    FVector delta = Velocity * deltaTime;
    FHitResult hit(1.f);
    SafeMoveUpdatedComponent(delta, rotation, true, hit);
    if (hit.IsValidBlockingHit())
    {
        SlideAlongSurface(delta, 1.f-hit.Time, hit.Normal, hit, true);
    }
}

void UPlanarMovement::AddVelocity(const FVector& addVelocity)
{
    SetVelocity(Velocity + addVelocity);
}

void UPlanarMovement::SetVelocity(const FVector& newVelocity)
{
    if (newVelocity.IsNearlyZero())
    {
        StopMovementImmediately();
    }
    else
    {
        Velocity = newVelocity;
    }
    UpdateComponentVelocity();
}

void UPlanarMovement::AddRotationSpeed(FRotator addRotationSpeed)
{
    SetRotationSpeed(mRotationSpeed + addRotationSpeed);
}

void UPlanarMovement::SetRotationSpeed(FRotator newRotationSpeed)
{
    mRotationSpeed = newRotationSpeed;
}
