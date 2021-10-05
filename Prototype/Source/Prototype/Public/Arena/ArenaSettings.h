#pragma once


#include "CoreMinimal.h"
#include "Arena/Damage.h"
#include "Engine/DeveloperSettings.h"

#include "ArenaSettings.generated.h"


UCLASS(Config = Game, DefaultConfig, Meta = ( DisplayName = "Arena" ))
class UArenaSettings
    : public UDeveloperSettings
{
    GENERATED_BODY()

public:

    UPROPERTY(Config, EditAnywhere, Category = "Movement Conflicts", Meta = (DisplayName = "Push Damage"))
    FDamageParameters mMovementConflictsPushDamage;

    UPROPERTY(Config, EditAnywhere, Category = "Movement Conflicts", Meta = (DisplayName = "Ram Damage"))
    FDamageParameters mMovementConflictsRamDamage;

public:

    static UArenaSettings* Get();

};