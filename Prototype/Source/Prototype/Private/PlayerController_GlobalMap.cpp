// Fill out your copyright notice in the Description page of Project Settings.

#include "PlayerController_GlobalMap.h"
#include "GameMode_GlobalMap.h"
#include "GMPawn.h"
#include "GMCamp.h"
#include "ProgressManager.h"
#include "GameInstance_Unknown.h"
#include "Kismet/GameplayStatics.h"
#include "UnknownCommon.h"


APlayerController_GlobalMap::APlayerController_GlobalMap()
    : mDefaultActionBlockID("Default")
    , mIsInputLocked(true)
{

}

void APlayerController_GlobalMap::BeginPlay()
{
    Super::BeginPlay();

    bShowMouseCursor = true;
    bEnableClickEvents = true;
    bEnableMouseOverEvents = true;

    mpGameMode = UUnknownCommon::GetGlobalMapGameMode(this);
    check(mpGameMode != nullptr && "PlayerController_GlobalMap is used without GameMode_GlobalMap.");
    mpGameMode->mEventStartedExecutingActionScripts.BindUObject(this, &APlayerController_GlobalMap::OnStartedExecutingActionScripts);
    mpGameMode->mEventFinishedExecutingActionScripts.BindUObject(this, &APlayerController_GlobalMap::OnFinishedExecutingActionScripts);

    mIsInputLocked = mpGameMode->IsExecutingActionScript();

    /*//  ~?~ Load state ideally should be called only once. Doing it into Posses is incorrect as we could change possession during the game.
    //  Here we are not 100% sure that BeginPlay will be called after player will be spawned. SHOULD WE SPAWN IT OURSELVES, not using the default pawn?
    if (UUnknownCommon::GetGameInstanceUnknown(this)->mLoadGlobalMapState)
    {
        
    }*/

    //  ~UGLY~ SetReady functions are not preferable. In this case no other solution was found.
    mpGameMode->SetPlayerControllerReady();
}

void APlayerController_GlobalMap::OnPossess(APawn* pPawn)
{
    Super::OnPossess(pPawn);

    mpGMPawn = Cast<AGMPawn>(pPawn);
    check(mpGMPawn != nullptr && "PlayerController_GlobalMap possessed pawn is not GMCharacter.");
    mpGMPawn->mEventCampChanged.BindUObject(this, &APlayerController_GlobalMap::OnCampChanged);
    mpGMPawn->mEventTravelFinished.BindUObject(this, &APlayerController_GlobalMap::OnTravelFinished);
    mpGMPawn->mEventMembersChanged.BindUObject(this, &APlayerController_GlobalMap::OnPartyMembersChanged);
    mNextActionBlockID = mDefaultActionBlockID;
    mpGMPawn->SetCurrentCamp(UUnknownCommon::GetGlobalMapGameMode(this)->GetStartCamp());

    mEventPartyChanged.Broadcast(mpGMPawn);
}

void APlayerController_GlobalMap::OnUnPossess()
{
    if (mpGMPawn != nullptr)
    {
        mpGMPawn->mEventCampChanged.Unbind();
        mpGMPawn->mEventTravelFinished.Unbind();
        mpGMPawn->mEventMembersChanged.Unbind();
    }

    Super::OnUnPossess();

    mpGMPawn = nullptr;
    mEventPartyChanged.Broadcast(mpGMPawn);
}

void APlayerController_GlobalMap::SaveState(FGMPlayerState& state)
{
    check(IsTraveling() == false && "Save operation is impossible while traveling.");
    state.mCurrentCampID = mpGMPawn->GetCurrentCamp()->GetCampID();
    check(mCurrentActionBlock.ID.IsNone() == false && "CurrentActionBlockID shouldn't be NONE.");
    state.mCurrentActionBlockID = mCurrentActionBlock.ID;
    state.mNextActionBlockID = mNextActionBlockID;
}

void APlayerController_GlobalMap::LoadState(FGMPlayerState& state)
{
    AGameMode_GlobalMap* pGameMode = UUnknownCommon::GetGlobalMapGameMode(this);
    AGMCamp* pCurrentCamp = pGameMode->TryGetCamp(state.mCurrentCampID);
    if (pCurrentCamp == nullptr)
    {
#if !UE_BUILD_SHIPPING
        UE_LOG(GlobalMapCommonLog, Warning, TEXT("Invalid CurrentCamp was saved: %s"), *state.mCurrentCampID.ToString());
#endif // #if !UE_BUILD_SHIPPING
        pCurrentCamp = pGameMode->GetStartCamp();
    }

    //  ~HACK~
    //  To recover previous state of the GM we need to switch to the saved "current camp".
    //  Additionally we have to switch to the saved "current AB" through setting NextActionBlockID.
    //  After that we set NextActionBlockID again to switch to it after all AS being executed:
    if (state.mCurrentActionBlockID.IsNone())
    {
#if !UE_BUILD_SHIPPING
        UE_LOG(GlobalMapCommonLog, Warning, TEXT("Invalid CurrentActionBlockID was saved: %s"), *state.mCurrentActionBlockID.ToString());
#endif // #if !UE_BUILD_SHIPPING
        mNextActionBlockID = mDefaultActionBlockID;
    }
    else
    {
        mNextActionBlockID = state.mCurrentActionBlockID;
    }
    mpGMPawn->SetCurrentCamp(pCurrentCamp);
    mNextActionBlockID = state.mNextActionBlockID;
}

AGMPawn* APlayerController_GlobalMap::GetGMPawn()
{
    return mpGMPawn;
}

void APlayerController_GlobalMap::StartCampTravel(AGMCamp* pTargetCamp, ECampTravel campTravelType, const FName& actionBlockID)
{
    check(actionBlockID.IsNone() == false && "While traveling first action block ID shouldn't be none!");
    mNextActionBlockID = actionBlockID;
    mpGMPawn->StartTravel(pTargetCamp, campTravelType);
}

void APlayerController_GlobalMap::StartCampTravelByID(const FName& targetCampID, ECampTravel campTravelType, const FName& actionBlockID)
{
    if (targetCampID.IsNone() == false &&
        targetCampID != GetCurrentCamp()->GetCampID())
    {
        auto pGameMode = Cast<AGameMode_GlobalMap>(UGameplayStatics::GetGameMode(this));
        check(pGameMode != nullptr && "PlayerController_GlobalMap is used without GameMode_GlobalMap.");
        AGMCamp* pTargetCamp = pGameMode->TryGetCamp(targetCampID);
#if !UE_BUILD_SHIPPING
        if (pTargetCamp == nullptr)
        {
            UE_LOG(GlobalMapCommonLog, Error, TEXT("Trying to switch to an unexisting camp %s."), *targetCampID.ToString());
        }
#endif // #if !UE_BUILD_SHIPPING
        StartCampTravel(pTargetCamp, campTravelType, actionBlockID);
    }
    else
    {
        SwitchActionBlock(actionBlockID);
    }
}

bool APlayerController_GlobalMap::IsTraveling() const
{
    return mpGMPawn->IsTraveling();
}

AGMCamp* APlayerController_GlobalMap::GetCurrentCamp() const
{
    return IsTraveling() ? nullptr : mpGMPawn->GetCurrentCamp();
    //return IsTraveling() ? nullptr : mpCurrentCamp;
}

const FActionBlock& APlayerController_GlobalMap::GetCurrentActionBlock() const
{
    return mCurrentActionBlock;
}

bool APlayerController_GlobalMap::IsInputLocked()
{
    return mIsInputLocked;// mpGameMode->IsInputLocked();
}

void APlayerController_GlobalMap::OnVariantSelected(const FActionVariant& actionVariant)
{
    UProgressManager* pProgressManager = UUnknownCommon::GetProgressManager(this);
    for (auto& result : actionVariant.Results)
    {
        bool doesExist = false;
        pProgressManager->ChangeProgressVariable(result, doesExist);
    }

    mNextActionBlockID = actionVariant.NextActionBlockID;

    //  If any action scripts are present - do not switch to the next camp and AB:
    if (actionVariant.ActionScripts.Num() > 0)
    {
        mpGameMode->EnqueueActionScripts(actionVariant.ActionScripts);
    }
    else
    {
        SwitchActionBlock(mNextActionBlockID);
    }
}

void APlayerController_GlobalMap::OnTravelFinished()
{
    mEventTravelFinished.Broadcast();
}

void APlayerController_GlobalMap::OnCampChanged(AGMCamp* pOldCamp, AGMCamp* pNewCamp)
{
    //what to do here? If we are traveling should we switch to mNextActionBlockID ?
    if (pOldCamp != nullptr)
    {
        pOldCamp->SetActive(false);
        pOldCamp->mEventRoadSelected.Unbind();
    }
    if (pNewCamp != nullptr)
    {
        pNewCamp->SetActive(true);
        pNewCamp->mEventRoadSelected.BindUObject(this, &APlayerController_GlobalMap::OnRoadSelected);
        SwitchActionBlock(mNextActionBlockID);
    }
    else
    {
        SetCurrentActionBlock(mSmthWentWrongActionBlock);
    }
}

void APlayerController_GlobalMap::OnRoadSelected(AGMRoad* pRoad, AGMCamp* pTargetCamp)
{
    if (IsInputLocked())
    {
#if !UE_BUILD_SHIPPING
        UE_LOG(GlobalMapCommonLog, Warning, TEXT("WARNING: RoadSelected event is fired while the input is blocked."));
#endif // #if !UE_BUILD_SHIPPING
        return;
    }
    StartCampTravel(pTargetCamp, ECampTravel::CampTravel_Neighbour, mDefaultActionBlockID);
    //mNextActionBlockID = mDefaultActionBlockID;
    //mpGMPawn->StartTravel(pTargetCamp, ECampTravel::CampTravel_Neighbour);
}

void APlayerController_GlobalMap::OnStartedExecutingActionScripts()
{
    mIsInputLocked = true;
}

void APlayerController_GlobalMap::OnFinishedExecutingActionScripts()
{
    mIsInputLocked = false;
    //  After all action scripts are executed we need to switch to the next action block:
    SwitchActionBlock(mNextActionBlockID);
}

void APlayerController_GlobalMap::SwitchActionBlock(const FName& actionBlockID)
{
    check(GetCurrentCamp() != nullptr && "Switching action block while current camp is not set.");
    const FCampData& campData = GetCurrentCamp()->GetCampData();
    if (campData.ActionBlocks.Num() == 0)
    {
#if !UE_BUILD_SHIPPING
        UE_LOG(GlobalMapCommonLog, Error, TEXT("No AB was found for current camp %s"), *GetCurrentCamp()->GetCampID().ToString());
#endif // #if !UE_BUILD_SHIPPING
        SetCurrentActionBlock(mSmthWentWrongActionBlock);
    }
    else if (actionBlockID.IsNone() || 
             actionBlockID == mCurrentActionBlock.ID)
    {
#if !UE_BUILD_SHIPPING
        UE_LOG(GlobalMapCommonLog, Warning, TEXT("Switching to the same AB %s. Current camp is %s"), *actionBlockID.ToString(), *GetCurrentCamp()->GetCampID().ToString());
#endif // #if !UE_BUILD_SHIPPING
    }
    //  Try to switch to a default action block of the current camp:
    else if (actionBlockID == mDefaultActionBlockID)
    {
        //  Firstly we check for the First Turn Dialogue blocks:
        if (campData.FirstTurnDialogues.Num() > 0)
        {
            UProgressManager* pProgressManager = UUnknownCommon::GetProgressManager(this);
            for (const FFirstTurnDialogue& firstTurnDialogue : campData.FirstTurnDialogues)
            {
                if (pProgressManager->MultiCheckProgressVariable(firstTurnDialogue.Conditions))
                {
                    SwitchActionBlock(firstTurnDialogue.ActionBlockID);
                    return;
                }
            }
        }
        //  If FTD was not set - set default AB:
        if (mCurrentActionBlock.ID != campData.ActionBlocks[0].ID)
        {
            SetCurrentActionBlock(campData.ActionBlocks[0]);
        }
    }
    else
    {
        FActionBlock actionBlock = {};
        if (GetCurrentCamp()->TryGetActionBlock(actionBlockID, actionBlock))
        {
            SetCurrentActionBlock(actionBlock);
        }
        else
        {
#if !UE_BUILD_SHIPPING
            UE_LOG(GlobalMapCommonLog, Error, TEXT("Failed to switch AB to %s. Current camp is %s"), *actionBlockID.ToString(), *GetCurrentCamp()->GetCampID().ToString());
#endif // #if !UE_BUILD_SHIPPING
            SetCurrentActionBlock(mSmthWentWrongActionBlock);
        }
    }
}

void APlayerController_GlobalMap::SetCurrentActionBlock(const FActionBlock& actionBlock)
{
    mCurrentActionBlock = actionBlock;
    //  ~?~ mNextActionBlockID = mCurrentActionBlock;
    FilterActiveVariants(mCurrentActionBlock);
    GenerateSystemVariants(mCurrentActionBlock);
    mEventActionBlockChanged.Broadcast(mCurrentActionBlock);
}

void APlayerController_GlobalMap::FilterActiveVariants(FActionBlock& actionBlock)
{
    TArray<int32> indicesToRemove;
    UProgressManager* pProgressManager = UUnknownCommon::GetProgressManager(this);
    actionBlock.Variants.RemoveAll([pProgressManager](const FActionVariant& actionVariant){
        return pProgressManager->MultiCheckProgressVariable(actionVariant.Conditions) == false;
    });
}

void APlayerController_GlobalMap::GenerateSystemVariants(FActionBlock& actionBlock)
{
    if (actionBlock.CanLeaveCamp)
    {
        actionBlock.Variants.Add(mVariantLeaveCamp);
    }
    //  ~TODO~
}

void APlayerController_GlobalMap::PostInitializeComponents()
{
    Super::PostInitializeComponents();
}

void APlayerController_GlobalMap::OnPartyMembersChanged()
{
    mEventPartyChanged.Broadcast(mpGMPawn);
}