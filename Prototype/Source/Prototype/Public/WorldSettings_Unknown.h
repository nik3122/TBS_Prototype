#pragma once


#include "Arena/ArenaSetupInfo.h"
#include "WorldSettings_Unknown.generated.h"


UCLASS()
class AWorldSettings_Unknown
    : public AWorldSettings
{
    GENERATED_BODY()

public:

    UPROPERTY(EditAnywhere, Category = "Unknown|Arena")
    EArenaGameMode mDefaultArenaGameMode = EArenaGameMode::INVALID;
};