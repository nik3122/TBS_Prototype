
#include "Arena/Weather.h"
#include "Arena/ArenaUtilities.h"
#include "Arena/ArenaManager_Interface.h"
#include "Arena/ArenaManager.h"
#include "Arena/GameState_Arena.h"
#include "GameInstance_Unknown.h"
#include "UnknownCommon.h"


void AWeather::SetupSystem()
{
    //  ~!~?~TODO~ Setup weather counting the global map weather.
    mWeatherState = UUnknownCommon::GetGameInstanceUnknown(this)->mWeatherState;
}

const FWeatherState& AWeather::GetWeatherState() const
{
    return mWeatherState;
}

bool AWeather::GenerateRangeAttackMiss() const
{
    return (FMath::Rand() % 10000) > mWeatherState.mPossibilityToMissRangeAttack;
}

void AWeather::BeginPlay()
{
    Super::BeginPlay();

    UArenaUtilities::GetArenaManager(this)->RegisterSystem(this);
}
