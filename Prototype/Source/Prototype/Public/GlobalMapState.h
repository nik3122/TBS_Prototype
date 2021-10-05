
#pragma once


#include "GlobalMapState.generated.h"


USTRUCT(BlueprintType, Category = "Unknown|GlobalMap")
struct FGlobalMapPlayerState
{
    GENERATED_BODY()

public:

    UPROPERTY(BlueprintReadWrite, Category = "Unknown|GlobalMap")
    FName mCurrentCampID;

    UPROPERTY(BlueprintReadWrite, Category = "Unknown|GlobalMap")
    FName mCurrentActionBlockID = "Default";

    UPROPERTY(BlueprintReadWrite, Category = "Unknown|GlobalMap")
    FName mNextActionBlockID = "Default";
};


USTRUCT(BlueprintType, Category = "Unknown|GlobalMap")
struct FGlobalMapState
{
    GENERATED_BODY()

public:

    UPROPERTY(BlueprintReadWrite, Category = "Unknown|GlobalMap")
    FGlobalMapPlayerState mPlayerState;
};