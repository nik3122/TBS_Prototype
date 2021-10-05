#pragma once


#include "ArenaSetupInfo.generated.h"


class ACreatureBase;


UENUM(BlueprintType)
enum class EArenaGameMode : uint8
{
    HotSeat,
    PvE,
    PvP,
    INVALID,
};


USTRUCT(BlueprintType)
struct FArenaSetupInfo
{
    GENERATED_BODY()

public:

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Arena")
    EArenaGameMode mArenaGameMode = EArenaGameMode::INVALID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Arena")
    FString mPartyOneName = "None 1";

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Arena")
    FLinearColor mPartyOneColor = { 1.0f, 0.01f, 0.01f };

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Arena")
    TArray<TSubclassOf<ACreatureBase>> mPartyOneMembers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Arena")
    FString mPartyTwoName = "None 2";

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Arena")
    FLinearColor mPartyTwoColor = { 0.0f, 0.2f, 1.0f };

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Arena")
    TArray<TSubclassOf<ACreatureBase>> mPartyTwoMembers;
};
