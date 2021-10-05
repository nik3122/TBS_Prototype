#pragma once

#include "GameFramework/SaveGame.h"
#include "CampaignSaveGame.generated.h"


USTRUCT()
struct FGMPlayerState
{
    GENERATED_BODY()

public:

    UPROPERTY()
    FName mCurrentCampID = "Default";

    UPROPERTY()
    FName mCurrentActionBlockID = "Default";

    UPROPERTY()
    FName mNextActionBlockID = "Default";

public:

    void ResetState()
    {
        mCurrentCampID = FName();
        mCurrentActionBlockID = FName();
        mNextActionBlockID = FName();
    }
};


USTRUCT()
struct FGMRoadState
{
    GENERATED_BODY()

public:

    UPROPERTY()
    FName mRoadID;

    UPROPERTY()
    bool mIsAvailable = true;
};


USTRUCT()
struct FGMCampState
{
    GENERATED_BODY()

public:

    UPROPERTY()
    FName mCampID;
};


USTRUCT()
struct FGMPawnState
{
    GENERATED_BODY()

public:

    //UPROPERTY();
};


USTRUCT()
struct FProgressManagerState
{
    GENERATED_BODY()

public:

    UPROPERTY()
    TMap<FName, int32> mActiveProgressVariables;

public:

    void ResetState()
    {
        mActiveProgressVariables.Empty();
    }
};


UCLASS()
class PROTOTYPE_API UCampaignSaveGame
    : public USaveGame
{
    GENERATED_BODY()

public:

    UPROPERTY()
    FProgressManagerState mProgressManagerState;

    UPROPERTY()
    TArray<FGMRoadState> mRoadsStates;

    UPROPERTY()
    TArray<FGMCampState> mCampsStates;

    UPROPERTY()
    FGMPlayerState mPlayerState;

    UPROPERTY()
    FGMPawnState mPlayerPawnState;

public:

    void ResetState()
    {
        mProgressManagerState.ResetState();
        mRoadsStates.Empty();
        mCampsStates.Empty();
        mPlayerState.ResetState();
    }
};