#pragma once


#include "StatusEffect.h"
#include "GameFramework\ProjectileMovementComponent.h"
#include "StatusEffect_Infection.generated.h"


UCLASS(Blueprintable)
class AStatusEffect_Infection
    : public AStatusEffect
{
    GENERATED_BODY()

public:

    void Apply(ETimeOfApplication timeOfApplication) override;

    void EndPlay(EEndPlayReason::Type endPlayReason) override;

    void Setup(UStatusEffectsManager* pStatusEffectsManager,
               AActor* pTargetActor) override;

    int32 GetLastTurnInfected() const;

private:

    int32 mLastTurnInfected = -1;
};