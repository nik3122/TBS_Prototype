#pragma once


#include "GameFramework/MovementComponent.h"
#include "PlanarMovement.generated.h"


UCLASS(BlueprintType, meta=(BlueprintSpawnableComponent))
class PROTOTYPE_API UPlanarMovement
    : public UMovementComponent
{
    GENERATED_BODY()

public:

    UPlanarMovement(const FObjectInitializer& objInit);

    void TickComponent(float deltaTime, ELevelTick tickType, FActorComponentTickFunction* pThisTickFunction) override;

    UFUNCTION(BlueprintCallable, Category = "Unknown")
    void AddVelocity(const FVector& addVelocity);

    UFUNCTION(BlueprintCallable, Category = "Unknown")
    void SetVelocity(const FVector& newVelocity);

    UFUNCTION(BlueprintCallable, Category = "Unknown")
    void AddRotationSpeed(FRotator addRotationSpeed);

    UFUNCTION(BlueprintCallable, Category = "Unknown")
    void SetRotationSpeed(FRotator newRotationSpeed);

private:

    FRotator mRotationSpeed;

};