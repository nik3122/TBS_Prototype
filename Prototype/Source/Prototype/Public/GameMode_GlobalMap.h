// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DialogueSystem.h"
#include "Engine/DataTable.h"
#include "GameFramework/GameModeBase.h"
#include "GlobalMapState.h"
#include "GameMode_GlobalMap.generated.h"


class APassMap;
class AGMCamp;
class AGMRoad;
class UGameInstance_Unknown;
class AActionScript_Base;


USTRUCT(BlueprintType)
struct FGlobalMapData
    : public FTableRowBase
{
    GENERATED_BODY()

public:

    UPROPERTY(EditAnywhere)
    UDataTable* mpCampsData;

    UPROPERTY(EditAnywhere)
    UDataTable* mpProgressVariables;
    
    UPROPERTY(EditAnywhere)
    UDataTable* mpScenarios_LoadArena;

    UPROPERTY(EditAnywhere)
    UDataTable* mpScenarios_CampTravel;

    UPROPERTY(EditAnywhere)
    UDataTable* mpScenarios_ManageParty;

    UPROPERTY(EditAnywhere)
    UDataTable* mpScenarios_ManipulateRoad;
};


UCLASS()
class PROTOTYPE_API AGameMode_GlobalMap
    : public AGameModeBase
{
    GENERATED_BODY()
	
public:

    DECLARE_DELEGATE(FEventLockInput);
    FEventLockInput mEventLockInput;

    DECLARE_DELEGATE(FEventUnlockInput);
    FEventUnlockInput mEventUnlockInput;

    DECLARE_DELEGATE(FEventStartedExecutingActionScripts);
    FEventStartedExecutingActionScripts mEventStartedExecutingActionScripts;

    DECLARE_DELEGATE(FEventFinishedExecutingActionScripts);
    FEventFinishedExecutingActionScripts mEventFinishedExecutingActionScripts;

public:

    AGameMode_GlobalMap();

    void BeginPlay() override;

    void InitGame(const FString& mapName,
                  const FString& options,
                  FString& errorMessage) override;

    UFUNCTION(BlueprintCallable, Category = "Unknown|GlobalMap")
    void SaveState();

    void LoadState();

    //PlayerControllerReady();

    UFUNCTION(BlueprintCallable, Category = "Unknown|GlobalMap")
    void StartArena(const FName& arenaLevelName);

    UDataTable* GetCampDataTable();

    AGMCamp* TryGetCamp(const FName& campID);

    AGMRoad* TryGetRoad(const FName& roadID);

    AGMRoad* TryGetRoad(const FName& startCampID, const FName& endCampID);

    void EnqueueActionScripts(const TArray<FActionScriptData>& actionScriptDatas);

    bool IsExecutingActionScript() const;

    AGMCamp* GetStartCamp() const;

    void SetPlayerControllerReady();

    bool CanSaveCampaignNow();

protected:

    UPROPERTY(EditDefaultsOnly, Category = "Unknown|ActionScript")
    TArray<TSubclassOf<AActionScript_Base>> mActionScriptTypes;

    UPROPERTY(EditDefaultsOnly, Category = "Unknown|GlobalMap")
    UDataTable* mpGlobalMapsData;

private:

    UGameInstance_Unknown* mpGameInstance;

    UDataTable* mpCampsDataTable;

    UDataTable* mpProgressVariablesTable;

    TMap<FName, AGMCamp*> mCamps;

    TMap<FName, AGMRoad*> mRoads;

    TMap<FName, AActionScript_Base*> mActionScripts;

    bool mIsExecutingActionScript;

    const FName mStartCampTag;

    AGMCamp* mpStartCamp;

    bool mIsPlayerControllerReady;

private:

    void ConstructActionScripts(FGlobalMapData& globalMapData);

    void OnActionScriptExecuted();

    void TryPickNextActionScript();

    void ExecuteActionScript(const FActionScriptData& actionScriptData);
};
