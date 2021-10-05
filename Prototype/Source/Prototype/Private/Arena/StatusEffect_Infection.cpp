

#include "Arena/StatusEffect_Infection.h"
#include "Arena/ArenaUtilities.h"
#include "Arena/InfectionSystem.h"
#include "Arena/TurnsManager.h"


void AStatusEffect_Infection::Apply(ETimeOfApplication timeOfApplication)
{
    mLastTurnInfected = mTurnsTimer > 0 ? UArenaUtilities::GetTurnsManager(this)->GetCurrentTurn() : mLastTurnInfected;

    Super::Apply(timeOfApplication);
}

void AStatusEffect_Infection::EndPlay(EEndPlayReason::Type endPlayReason)
{
    Super::EndPlay(endPlayReason);

    if (endPlayReason == EEndPlayReason::Destroyed ||
        endPlayReason == EEndPlayReason::RemovedFromWorld)
    {
        UArenaUtilities::GetInfectionSystem(this)->UnregisterInfectable(this);
    }
}

void AStatusEffect_Infection::Setup(UStatusEffectsManager* pStatusEffectsManager,
                                    AActor* pTargetActor)
{
    Super::Setup(pStatusEffectsManager, pTargetActor);

    UArenaUtilities::GetInfectionSystem(this)->RegisterInfectable(this);
}

int32 AStatusEffect_Infection::GetLastTurnInfected() const
{
    return mLastTurnInfected;
}
