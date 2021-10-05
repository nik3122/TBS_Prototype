// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "DialogueSystem.h"
#include "CampaignSaveGame.h"
#include "PlayerController_GlobalMap.generated.h"


class AGameMode_GlobalMap;
class AGMCamp;
class AGMPawn;
class AGMRoad;


UCLASS(Blueprintable, Category = "Unknown|GlobalMap")
class APlayerController_GlobalMap
    : public APlayerController
{
	GENERATED_BODY()

public:

    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FEventCampChanged, AGMCamp*, pNewCamp);
    UPROPERTY(BlueprintAssignable, Category = "Unknown|GlobalMap")
    FEventCampChanged mEventCampChanged;

    DECLARE_DYNAMIC_MULTICAST_DELEGATE(FEventTravelFinished);
    UPROPERTY(BlueprintAssignable, Category = "Unknown|GlobalMap")
    FEventTravelFinished mEventTravelFinished;

    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FEventActionBlockChanged, FActionBlock, newActionBlock);
    UPROPERTY(BlueprintAssignable, Category = "Unknown|GlobalMap")
    FEventActionBlockChanged mEventActionBlockChanged;

    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FEventPartyChanged, AGMPawn*, pGMPawn);
    UPROPERTY(BlueprintAssignable, Category = "Unknown|GlobalMap")
    FEventPartyChanged mEventPartyChanged;

public:

    APlayerController_GlobalMap();

    void BeginPlay() override;

    void PostInitializeComponents() override;

    void OnPossess(APawn* pPawn) override;

    void OnUnPossess() override;

    void SaveState(FGMPlayerState& state);

    void LoadState(FGMPlayerState& state);

    UFUNCTION(BlueprintPure, Category = "Unknown|GlobalMap")
    AGMPawn* GetGMPawn();

    UFUNCTION(BlueprintCallable, Category = "Unknown|GlobalMap")
    void StartCampTravel(AGMCamp* pTargetCamp, ECampTravel campTravelType, const FName& actionBlockID);

    UFUNCTION(BlueprintCallable, Category = "Unknown|GlobalMap")
    void StartCampTravelByID(const FName& targetCampID, ECampTravel campTravelType, const FName& actionBlockID);

    UFUNCTION(BlueprintPure, Category = "Unknown|GlobalMap")
    bool IsTraveling() const;

    UFUNCTION(BlueprintPure, Category = "Unknown|GlobalMap")
    AGMCamp* GetCurrentCamp() const;

    UFUNCTION(BlueprintCallable, Category = "Unknown|GlobalMap")
    void SwitchActionBlock(const FName& actionBlockID);

    UFUNCTION(BlueprintPure, Category = "Unknown|GlobalMap")
    const FActionBlock& GetCurrentActionBlock() const;

    UFUNCTION(BlueprintPure, Category = "Unknown|GlobalMap")
    bool IsInputLocked();

    UFUNCTION(BlueprintCallable, Category = "Unknown|GlobalMap")
    void OnVariantSelected(const FActionVariant& actionVariant);

protected:

    UPROPERTY(BlueprintReadOnly, Category = "Unknown|GlobalMap")
    AGMPawn* mpGMPawn;

    UPROPERTY(BlueprintReadOnly, Category = "Unknown|GlobalMap")
    FActionBlock mCurrentActionBlock;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Unknown|GlobalMap")
    FActionBlock mSmthWentWrongActionBlock;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Unknown|GlobalMap")
    FName mDefaultActionBlockID;

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Unknown|GlobalMap")
    FActionVariant mVariantLeaveCamp;

private:

    //bool mIsResetCampNeeded;

    bool mIsInputLocked;

    AGameMode_GlobalMap* mpGameMode;

    /*
        NextActionBlockID determines what action block should be loaded after action script execution or after camp change.
        It is updated at the next places:
            1) On loading state - determines which AB should be set at the start of the game.
            2) On camp travel - determines which AB should be set after reaching target camp.
            3) On road selection - the same as camp travel.
            4) On variant selection - determines which AB should be switched after variant being processed.
    */
    FName mNextActionBlockID;

    TArray<FName> mPartyMembers;

    int32 mPartyMembersCountLimit;

private:

    void OnTravelFinished();

    void OnCampChanged(AGMCamp* pOldCamp, AGMCamp* pNewCamp);

    void OnRoadSelected(AGMRoad* pRoad, AGMCamp* pTargetCamp);

    void OnStartedExecutingActionScripts();

    void OnFinishedExecutingActionScripts();

    void SetCurrentActionBlock(const FActionBlock& actionBlock);

    void FilterActiveVariants(FActionBlock& actionBlock);

    void GenerateSystemVariants(FActionBlock& actionBlock);

    void OnPartyMembersChanged();

};
