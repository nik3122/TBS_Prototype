// Fill out your copyright notice in the Description page of Project Settings.

#include "GameMode_GlobalMap.h"
#include "Kismet/GameplayStatics.h"
#include "GameInstance_Unknown.h"
#include "PlayerController_GlobalMap.h"
#include "ProgressManager.h"
#include "GMCamp.h"
#include "GMPawn.h"
#include "GMRoad.h"
#include "UnknownCommon.h"
#include "CampaignSaveGame.h"
#include "ActionScript_Base.h"
#include "ActionScript_LoadArena.h"
#include "ActionScript_CampTravel.h"
#include "ActionScript_ManageParty.h"
#include "ActionScript_ManipulateRoad.h"
#include "EngineUtils.h"


AGameMode_GlobalMap::AGameMode_GlobalMap()
    : mIsExecutingActionScript(true)
    , mStartCampTag("Start")
{
    PlayerControllerClass = APlayerController_GlobalMap::StaticClass();
    DefaultPawnClass = AGMPawn::StaticClass();
    PlayerStateClass = nullptr;
    GameSessionClass = nullptr;
    GameStateClass = nullptr;
    ReplaySpectatorPlayerControllerClass = nullptr;
    ServerStatReplicatorClass = nullptr;
    SpectatorClass = nullptr;
}

void AGameMode_GlobalMap::BeginPlay()
{
    Super::BeginPlay();

    LoadState();

    //  ~!~ ~HACK~ Its said that begin play is called for  Finaly try pick next action to execute:
    TryPickNextActionScript();
}

void AGameMode_GlobalMap::InitGame(const FString& mapName,
                                   const FString& options,
                                   FString& errorMessage)
{
    Super::InitGame(mapName,
                    options,
                    errorMessage);

    check(mpGlobalMapsData != nullptr && "Global map data table isn't set!");
    FName globalMapName = *UGameplayStatics::GetCurrentLevelName(this);
    FGlobalMapData* pGlobalMapData = mpGlobalMapsData->FindRow<FGlobalMapData>(globalMapName, "", false);
    check(pGlobalMapData != nullptr && "Global map data for current level is not set!");
    mpCampsDataTable = pGlobalMapData->mpCampsData;
    mpProgressVariablesTable = pGlobalMapData->mpProgressVariables;

    mpGameInstance = Cast<UGameInstance_Unknown>(GetGameInstance());
    mpGameInstance->GetProgressManager()->SetProgressVariablesTable(mpProgressVariablesTable);

    //  Create map of all existing camps:
    {
        TActorIterator<AGMCamp> campIter(GetWorld());
        while (campIter)
        {
            FName campID(campIter->GetFName());
            AGMCamp* pCamp = *campIter;
            mCamps.Add(campID, pCamp);
            if (pCamp->ActorHasTag(mStartCampTag))
            {
                mpStartCamp = pCamp;
            }
            ++campIter;
        }
    }

    //  Create map of all existing roads:
    {
        TActorIterator<AGMRoad> roadIter(GetWorld());
        while (roadIter)
        {
            FName roadID(roadIter->GetFName());
            AGMRoad* pRoad = *roadIter;
            mRoads.Add(roadID, pRoad);
            ++roadIter;
        }
    }

    ConstructActionScripts(*pGlobalMapData);
}

void AGameMode_GlobalMap::SaveState()
{
    //auto* pSaveGameObject = Cast<UCampaignSaveGame>(UGameplayStatics::CreateSaveGameObject(UCampaignSaveGame::StaticClass()));
    UCampaignSaveGame* pSaveGameObject = mpGameInstance->mpCampaignSaveGame;

    TArray<AGMRoad*> roads;
    mRoads.GenerateValueArray(roads);
    pSaveGameObject->mRoadsStates.SetNum(roads.Num());
    for (int32 i = 0; i < roads.Num(); ++i)
    {
        roads[i]->SaveState(pSaveGameObject->mRoadsStates[i]);
    }

    TArray<AGMCamp*> camps;
    mCamps.GenerateValueArray(camps);
    pSaveGameObject->mCampsStates.SetNum(camps.Num());
    for (int32 i = 0; i < camps.Num(); ++i)
    {
        camps[i]->SaveState(pSaveGameObject->mCampsStates[i]);
    }

    auto* pPlayerController = Cast<APlayerController_GlobalMap>(UGameplayStatics::GetPlayerController(this, 0));
    check(pPlayerController != nullptr && "Player controller should be PlayerController_GlobalMap.");
    pPlayerController->SaveState(pSaveGameObject->mPlayerState);
}

void AGameMode_GlobalMap::LoadState()
{
    UCampaignSaveGame* pSaveGameObject = mpGameInstance->mpCampaignSaveGame;

    //  Load roads state:
    {
        for (const FGMRoadState& roadState : pSaveGameObject->mRoadsStates)
        {
            AGMRoad** ppRoad = mRoads.Find(roadState.mRoadID);
            if (ppRoad != nullptr)
            {
                (*ppRoad)->LoadState(roadState);
            }
        }
    }

    //  Load camps state:
    {
        for (const FGMCampState& campState : pSaveGameObject->mCampsStates)
        {
            AGMCamp** ppCamp = mCamps.Find(campState.mCampID);
            if (ppCamp!= nullptr)
            {
                (*ppCamp)->LoadState(campState);
            }
        }
    }

    auto* pPlayerController = Cast<APlayerController_GlobalMap>(UGameplayStatics::GetPlayerController(this, 0));
    check(pPlayerController != nullptr && "Player controller should be PlayerController_GlobalMap.");
    pPlayerController->LoadState(pSaveGameObject->mPlayerState);
}

void AGameMode_GlobalMap::StartArena(const FName& arenaLevelName)
{
    //  ~!~ Don't worry that progress manager state is not saved - it's internal state isn't changed during transfer to arena and back.
    SaveState();
    //  ~?~ Is it better to wrap into GameInstance_Unknown::OpenArena(FName afterArenaLevelName)?
    mpGameInstance->mAfterArenaLevelName = UGameplayStatics::GetCurrentLevelName(this);
    UGameplayStatics::OpenLevel(this, arenaLevelName);
}

AGMCamp* AGameMode_GlobalMap::TryGetCamp(const FName& campID)
{
    AGMCamp** dpCamp = mCamps.Find(campID);
    return dpCamp == nullptr ? nullptr : *dpCamp;
}

AGMRoad* AGameMode_GlobalMap::TryGetRoad(const FName& roadID)
{
    AGMRoad** dpRoad = mRoads.Find(roadID);
    return dpRoad == nullptr ? nullptr : *dpRoad;
}

AGMRoad* AGameMode_GlobalMap::TryGetRoad(const FName& startCampID, const FName& endCampID)
{
    AGMCamp* pStartCamp = TryGetCamp(startCampID);
    if (pStartCamp != nullptr)
    {
        AGMCamp* pEndCamp = TryGetCamp(endCampID);
        if (pEndCamp != nullptr)
        {
            const TArray<AGMRoad*>& roads = pStartCamp->GetRoads();
            for (AGMRoad* pRoad : roads)
            {
                if (pEndCamp == pRoad->GetOppositeCamp(pStartCamp))
                {
                    return pRoad;
                }
            }
        }
    }
    return nullptr;
}
 
UDataTable* AGameMode_GlobalMap::GetCampDataTable()
{
    return mpCampsDataTable;
}

void AGameMode_GlobalMap::ConstructActionScripts(FGlobalMapData& globalMapData)
{
    /*for (auto& actionScriptClass : mActionScriptTypes)
    {
        AActionScript_Base* pCreatedActionScript = GetWorld()->SpawnActor<AActionScript_Base>(actionScriptClass);
        mActionScripts.Add(pCreatedActionScript->GetActionScriptID(), pCreatedActionScript);
        pCreatedActionScript->mEventExecutionFinished.BindUObject(this, &AGameMode_GlobalMap::OnActionScriptExecuted);
    }*/

    {
        auto* pCreatedActionScript = GetWorld()->SpawnActor<AActionScript_LoadArena>();
        pCreatedActionScript->Setup(globalMapData.mpScenarios_LoadArena);
        pCreatedActionScript->mEventExecutionFinished.BindUObject(this, &AGameMode_GlobalMap::OnActionScriptExecuted);
        mActionScripts.Add(pCreatedActionScript->GetActionScriptID(), pCreatedActionScript);
    }
    {
        auto* pCreatedActionScript = GetWorld()->SpawnActor<AActionScript_CampTravel>();
        pCreatedActionScript->Setup(globalMapData.mpScenarios_CampTravel);
        pCreatedActionScript->mEventExecutionFinished.BindUObject(this, &AGameMode_GlobalMap::OnActionScriptExecuted);
        mActionScripts.Add(pCreatedActionScript->GetActionScriptID(), pCreatedActionScript);
    }
    {
        auto* pCreatedActionScript = GetWorld()->SpawnActor<AActionScript_ManageParty>();
        pCreatedActionScript->Setup(globalMapData.mpScenarios_ManageParty);
        pCreatedActionScript->mEventExecutionFinished.BindUObject(this, &AGameMode_GlobalMap::OnActionScriptExecuted);
        mActionScripts.Add(pCreatedActionScript->GetActionScriptID(), pCreatedActionScript);
    }
    {
        auto* pCreatedActionScript = GetWorld()->SpawnActor<AActionScript_ManipulateRoad>();
        pCreatedActionScript->Setup(globalMapData.mpScenarios_ManipulateRoad);
        pCreatedActionScript->mEventExecutionFinished.BindUObject(this, &AGameMode_GlobalMap::OnActionScriptExecuted);
        mActionScripts.Add(pCreatedActionScript->GetActionScriptID(), pCreatedActionScript);
    }
}

void AGameMode_GlobalMap::OnActionScriptExecuted()
{
    TryPickNextActionScript();
}

void AGameMode_GlobalMap::EnqueueActionScripts(const TArray<FActionScriptData>& actionScriptDatas)
{
    check(actionScriptDatas.Num() > 0 && "Trying to enqueue no action scripts.");
    for (auto& actionScriptData : actionScriptDatas)
    {
        mpGameInstance->EnqueueActionScript(actionScriptData);
    }
    TryPickNextActionScript();
}

void AGameMode_GlobalMap::TryPickNextActionScript()
{
    if (HasActorBegunPlay() == false ||
        mIsPlayerControllerReady == false)
    {
        return;
    }
    FActionScriptData actionScriptData;
    if (mpGameInstance->DequeueActionScript(actionScriptData) == false)
    {
        if (mIsExecutingActionScript)
        {
            mEventFinishedExecutingActionScripts.ExecuteIfBound();
            mIsExecutingActionScript = false;
        }
    }
    else
    {
        if (mIsExecutingActionScript == false)
        {
            mEventStartedExecutingActionScripts.ExecuteIfBound();
            mIsExecutingActionScript = true;
        }
        ExecuteActionScript(actionScriptData);
    }
}

void AGameMode_GlobalMap::ExecuteActionScript(const FActionScriptData& actionScriptData)
{
    AActionScript_Base** dpActionScript = mActionScripts.Find(actionScriptData.ActionScriptID);
    if (dpActionScript == nullptr)
    {
#if !UE_BUILD_SHIPPING
        UE_LOG(GlobalMapCommonLog, Error, TEXT("ERROR: Action script \"%s\" doesn't exist!"), *actionScriptData.ActionScriptID.ToString());
#endif // #if !UE_BUILD_SHIPPING
        OnActionScriptExecuted();
        return;
    }
    (*dpActionScript)->Execute(actionScriptData.ScenarioID);
}

bool AGameMode_GlobalMap::IsExecutingActionScript() const
{
    return mIsExecutingActionScript;
}

AGMCamp* AGameMode_GlobalMap::GetStartCamp() const
{
    check(mpStartCamp != nullptr && "Start camp is not initialized.");
    return mpStartCamp;
}

void AGameMode_GlobalMap::SetPlayerControllerReady()
{
    mIsPlayerControllerReady = true;
    TryPickNextActionScript();
}

bool AGameMode_GlobalMap::CanSaveCampaignNow()
{
    //  ~TODO~
    return true;
}
