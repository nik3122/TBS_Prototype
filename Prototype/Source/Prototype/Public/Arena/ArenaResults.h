#pragma once


#include "ArenaResults.generated.h"


class AParty;


USTRUCT(BlueprintType)
struct FArenaResults
{
    GENERATED_BODY()

public:

    UPROPERTY(BlueprintReadWrite, Category = "Unknown|Arena")
    bool mIsPartyOneDead = false;

    UPROPERTY(BlueprintReadWrite, Category = "Unknown|Arena")
    bool mIsPartyTwoDead = false;

    //  nullptr - if draw.
    UPROPERTY(BlueprintReadWrite, Category = "Unknown|Arena")
    AParty* mpWinnerParty;
};
