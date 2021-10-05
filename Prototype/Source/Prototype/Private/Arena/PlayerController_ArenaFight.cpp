// Fill out your copyright notice in the Description page of Project Settings.

#include "Arena/PlayerController_ArenaFight.h"
#include "Arena/ActionManager.h"
#include "Arena/Action_SeparateMovement_Base.h"
#include "Arena/ArenaManager.h"
#include "Arena/ArenaUtilities.h"
#include "Arena/CreatureBase.h"
#include "Arena/Damageable.h"
#include "Arena/GameMode_ArenaFight.h"
#include "Arena/GameState_Arena.h"
#include "Arena/Party.h"
#include "Arena/PassMap.h"
#include "Arena/PlayerState_Arena.h"
#include "Arena/TiledMovement.h"
#include "Arena/TurnsManager.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "TileMaps/HexTilePattern.h"
#include "UnknownCommon.h"


APlayerController_ArenaFight::APlayerController_ArenaFight(const FObjectInitializer& objectInitializer)
    : Super(objectInitializer)
{
    bAutoManageActiveCameraTarget = false;
}

void APlayerController_ArenaFight::Tick(float deltaSeconds)
{
    Super::Tick(deltaSeconds);

    UpdateTileUnderCursor();

    ATurnsManager* turnsManager = UArenaUtilities::GetTurnsManager(this);
    bool isExecutionPhase = turnsManager->IsExecuting();

    if (mPassMap != nullptr &&
        mpFocusedMember != nullptr &&
        isExecutionPhase == false)
    {
        for (const FIntPoint& stepTile : mMovementPath)
        {
            FVector tileLocation = mPassMap->GetTilePositionWorld(stepTile);
            UKismetSystemLibrary::DrawDebugSphere(this,
                                                    tileLocation,
                                                    80.f,
                                                    16,
                                                    FLinearColor::Blue);
        }

        FIntPoint creatureTile = mpFocusedMember->GetTiledMovement()->GetCurrentTile();
        FIntPoint lastStepTile = mMovementPath.Num() > 0 ? mMovementPath.Last(0) : creatureTile;
        FVector lastStepTileLocation = mPassMap->GetTilePositionWorld(lastStepTile) + FVector(0.0f, 0.0f, 40.0f);
        FVector tileUnderCursorLocation = mPassMap->GetTilePositionWorld(mTileUnderCursor) + FVector(0.0f, 0.0f, 40.0f);
        int32 tileDirection = mPassMap->GetDirectionBetweenTiles(lastStepTile, mTileUnderCursor);
        FRotator directionRot = mPassMap->GetDirectionRotation(tileDirection);

        UKismetSystemLibrary::DrawDebugLine(this,
                                            lastStepTileLocation,
                                            lastStepTileLocation + directionRot.Vector() * 500.f,
                                            FLinearColor::Red,
                                            0.0f,
                                            3.0f);

        FRotator directionRotation = mPassMap->GetDirectionRotation(mExecutionDirection);
        FVector executionDirectionVector = directionRotation.Vector() * 200.0f;
        UKismetSystemLibrary::DrawDebugLine(this,
                                            lastStepTileLocation,
                                            lastStepTileLocation + executionDirectionVector,
                                            FLinearColor::Green,
                                            0.0f,
                                            5.0f);

        auto focusedAction = Cast<AAction_SeparateMovement_Base>(mFocusedAction);
        if (focusedAction != nullptr)
        {
            UHexTilePattern* tilePattern = focusedAction->GetTilePattern();
            if (tilePattern != nullptr)
            {
                TArray<FIntPoint> patternTiles = tilePattern->GetPatternTiles(lastStepTile, tileDirection);
                for (const FIntPoint& tile : patternTiles)
                {
                    FVector tileLoc = mPassMap->GetTilePositionWorld(tile);
                    UKismetSystemLibrary::DrawDebugBox(this,
                                                       tileLoc,
                                                       FVector(40.f, 40.f, 40.f),
                                                       FLinearColor(1.f, 0.3f, 0.1f));
                }
            }
        }
    }
}

void APlayerController_ArenaFight::InitPlayerState()
{
    Super::InitPlayerState();

    mpArenaPlayerState = Cast<APlayerState_Arena>(PlayerState);
}

APlayerState_Arena* APlayerController_ArenaFight::GetArenaPlayerState() const
{
    check(IsValid(mpArenaPlayerState));
    return mpArenaPlayerState;
}

AParty* APlayerController_ArenaFight::GetPossessedParty() const
{
    return mpPossessedParty;
}

void APlayerController_ArenaFight::Client_NotifyOpponentQuit_Implementation()
{
    UE_LOG(ArenaCommonLog, Log, TEXT("The opponent has left the game."));
    K2_NotifyOpponentQuit();
}

bool APlayerController_ArenaFight::GetIsPlayerControlLocked() const
{
    return mIsPlayerControlLocked;
}

void APlayerController_ArenaFight::BeginPlay()
{
    Super::BeginPlay();

    GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Red, TEXT("APlayerController_ArenaFight::BeginPlay()"));

    auto pArenaGameState = Cast<AGameState_Arena>(UGameplayStatics::GetGameState(this));
    pArenaGameState->mEventEndOfArena.AddDynamic(this, &APlayerController_ArenaFight::OnEndOfArena);

    UNKNOWN_NOT_ARENA_EDITOR_BEGIN(this)

        if (IsLocalPlayerController())
        {
            AArenaManager* pArenaManager = UArenaUtilities::GetArenaManager(this);
            if (pArenaManager->IsArenaSetup())
            {
                OnArenaSetup();
            }
            else
            {
                pArenaManager->mEventArenaSetup.BindUObject(this, &APlayerController_ArenaFight::OnArenaSetup);
            }
        }

    UNKNOWN_NOT_ARENA_EDITOR_END
}

void APlayerController_ArenaFight::OnPossess(APawn* pPossessedPawn)
{
    AParty* pParty = Cast<AParty>(pPossessedPawn);
    check(IsValid(pParty) && "APlayerController_ArenaFight can possess only AParty.");
    Super::OnPossess(pPossessedPawn);
    SetPossessedParty(pParty);
}

void APlayerController_ArenaFight::OnUnPossess()
{
    Super::OnUnPossess();
    SetPossessedParty(nullptr);
}

void APlayerController_ArenaFight::OnRep_Pawn()
{
    AParty* pParty = Cast<AParty>(GetPawn());
    SetPossessedParty(pParty);
}

void APlayerController_ArenaFight::OnRep_PlayerState()
{
    mpArenaPlayerState = Cast<APlayerState_Arena>(PlayerState);
    check(IsValid(mpArenaPlayerState) && "Player state must inherit APlayerArena_State.");
}

void APlayerController_ArenaFight::SetupInputComponent()
{
    Super::SetupInputComponent();

    InputComponent->BindAction(TEXT("ActionSetupPickMovementStepTile"), IE_Pressed, this, &APlayerController_ArenaFight::OnActionSetupPickMovementStepTile);
    InputComponent->BindAction(TEXT("ActionSetupPickMovementDestinationTile"), IE_Pressed, this, &APlayerController_ArenaFight::OnActionSetupPickMovementDestinationTile);
    InputComponent->BindAction(TEXT("ActionSetupPickExecutionDirection"), IE_Pressed, this, &APlayerController_ArenaFight::OnActionSetupPickExecutionDirection);
    InputComponent->BindAction(TEXT("ShowAllPartyMembersActionsSetup"), IE_Pressed, this, &APlayerController_ArenaFight::OnShowAllPartyMembersActionsSetup);
    InputComponent->BindAction(TEXT("ShowAllPartyMembersActionsSetup"), IE_Released, this, &APlayerController_ArenaFight::OnHideAllPartyMembersActionsSetup);
}

void APlayerController_ArenaFight::OnArenaSetup_Implementation()
{
    ATurnsManager* pTurnsManager = UArenaUtilities::GetTurnsManager(this);
    pTurnsManager->mEventPreExecution.AddDynamic(this, &APlayerController_ArenaFight::OnPreExecution);
    pTurnsManager->mEventUnlockPlayerControl.BindUObject(this, &APlayerController_ArenaFight::UnlockPlayerControl);

    mPassMap = UArenaUtilities::GetPassMap(this);

    //  In cases when it is Dedicated Server, Listen Server or Standalone there is no need to subscribe:
    if (GetNetMode() == ENetMode::NM_Client)
    {
        pTurnsManager->mEventTurnFinished.BindUObject(this, &APlayerController_ArenaFight::OnTurnFinished);
    }

    //  All clients including ListenServer must notify about it's arena setup:
    if (GetNetMode() != ENetMode::NM_DedicatedServer)
    {
        Server_NotifyArenaSetup();
    }
}

void APlayerController_ArenaFight::SetIsPlanningFinished(bool isFinished)
{
    if (isFinished)
    {
        LockPlayerControl();
    }
    Server_SetIsPlanningFinished(isFinished);
}

void APlayerController_ArenaFight::OnPreExecution()
{
    check(IsValid(mpArenaPlayerState));
    mpArenaPlayerState->mIsExecutionFinished = false;
}

void APlayerController_ArenaFight::OnTurnFinished()
{
    check(GetNetMode() != ENetMode::NM_DedicatedServer);
    Server_NotifyFinishedExecution();
}

void APlayerController_ArenaFight::SwitchHotSeatParty()
{
    auto pArenaGameState = Cast<AGameState_Arena>(UGameplayStatics::GetGameState(this));
    if (IsValid(pArenaGameState) && pArenaGameState->GetArenaGameMode() == EArenaGameMode::HotSeat)
    {
        Server_SwitchHotSeatParty();
    }
}

bool APlayerController_ArenaFight::Server_SwitchHotSeatParty_Validate()
{
    return UArenaUtilities::GetArenaGameState(this)->GetArenaGameMode() == EArenaGameMode::HotSeat;
}

void APlayerController_ArenaFight::Server_SwitchHotSeatParty_Implementation()
{
    TScriptInterface<IArenaManager_Interface> arenaManagerInterface;
    UArenaUtilities::GetArenaManagerInterface(this, arenaManagerInterface);
    AParty* pPartyOne = IArenaManager_Interface::Execute_GetPartyOne(arenaManagerInterface.GetObject());
    AParty* pPartyTwo = IArenaManager_Interface::Execute_GetPartyTwo(arenaManagerInterface.GetObject());
    AParty* pOldParty = mpPossessedParty;
    Possess(mpPossessedParty != pPartyOne ? pPartyOne : pPartyTwo);

    //  If hotseat is hold on the server the unpossessed party must stay owned by the player controller to be properly replicated:
    pOldParty->SetOwner(this);
}

void APlayerController_ArenaFight::SwitchFocusedMember(bool forwardIteration)
{
    if (mpPossessedParty == nullptr ||
        mpPossessedParty->IsNotEmpty() == false)
    {
        return;
    }

    int32 focusedMemberIndex = 0;
    bool isInParty = mpPossessedParty->IsInParty(mpFocusedMember, focusedMemberIndex);
    if (isInParty)
    {
        int32 nextFocusablaMemberIndex = 0;
        if (FindNextFocusableMember(focusedMemberIndex, forwardIteration, nextFocusablaMemberIndex) == false)
        {
            nextFocusablaMemberIndex = focusedMemberIndex;
        }
        if (isInParty == false)
        {
            UE_LOG(ArenaCommonLog, Verbose, TEXT("No alive member was detected while switch. Unfocus all members."));
        }
        SetFocusedMemberByIndex(nextFocusablaMemberIndex);
    }
    else
    {
        ResetFocusedMember();
    }
}

void APlayerController_ArenaFight::SetFocusedMember(ACreatureBase* pExistingMember)
{
    int32 memberIndex = 0;
    bool isInParty = mpPossessedParty->IsInParty(pExistingMember, memberIndex);
    check(isInParty && "Party doesn't contain a creature. Can't set unexisting member as focused.");
    SetFocusedMemberByIndex(memberIndex);
}

void APlayerController_ArenaFight::SetFocusedMemberByIndex(int32 memberIndex)
{
    ACreatureBase* oldFocused = mpFocusedMember;
    mpFocusedMember = mpPossessedParty->IsIndexValid(memberIndex) ? mpPossessedParty->GetMemberByIndex(memberIndex) : nullptr;
    mEventFocusedMemberChanged.Broadcast(oldFocused , mpFocusedMember);

    if (oldFocused != nullptr)
    {
        oldFocused->SetIsFocusedPartyMember(false);
    }

    if (mpFocusedMember != nullptr)
    {
        UActionManager* actionManager = mpFocusedMember->GetActionManager();
        const TArray<AAction_Base*>& allActions = actionManager->GetAllActions();
        AAction_Base* actionToExecute = actionManager->GetActionToExecute();
        //  ActionToExecute may be "Idle" or other special action, which is not listed among the regular actions.
        if (actionToExecute != nullptr &&
            allActions.Find(actionToExecute) != INDEX_NONE)
        {
            SetFocusedAction(actionToExecute);
        }
        else
        {
            SetFocusedAction(nullptr);
        }

        mpFocusedMember->SetIsFocusedPartyMember(true);
    }
}

void APlayerController_ArenaFight::GetFocusedMember(ACreatureBase*& pFocusedMember, int32& memberIndex)
{
    pFocusedMember = mpFocusedMember;
    mpPossessedParty->IsInParty(mpFocusedMember, memberIndex);
}

void APlayerController_ArenaFight::ResetFocusedMember(bool unfocusAll)
{
    int32 focusableMemberIndex = -1;
    if (IsValid(mpPossessedParty) &&
        mpPossessedParty->IsNotEmpty() &&
        unfocusAll == false)
    {
        FindNextFocusableMember(-1, true, focusableMemberIndex);
    }
    SetFocusedMemberByIndex(focusableMemberIndex);
}

void APlayerController_ArenaFight::OnMembersUpdated(AParty* pParty)
{
    checkSlow(mpPossessedParty == pParty && "OnMembersUpdated() is called while other party is possessed.");
    checkSlow(IsValid(mpPossessedParty) && "OnMembersUpdated() is called while no party is possessed.");

    ValidateFocusedMember();
    mEventMembersUpdated.Broadcast(mpPossessedParty);
}

void APlayerController_ArenaFight::SetPossessedParty(AParty* pNewParty)
{
    if (mpPossessedParty == pNewParty) return;
    AParty* pOldParty = mpPossessedParty;
    if (IsValid(pOldParty))
    {
        pOldParty->mEventPartyMembersUpdated.RemoveDynamic(this, &APlayerController_ArenaFight::OnMembersUpdated);
    }
    mpPossessedParty = pNewParty;
    ACreatureBase* pOldFocusedMember = mpFocusedMember;
    if (IsValid(pNewParty))
    {
        pNewParty->mEventPartyMembersUpdated.AddDynamic(this, &APlayerController_ArenaFight::OnMembersUpdated);
        mpFocusedMember = pNewParty->IsNotEmpty() ? pNewParty->GetMemberByIndex(0) : nullptr;
    }
    else
    {
        mpFocusedMember = nullptr;
    }
    //  ~!~ EventPartyChanged must be called before EventFocusedMemberChanged:
    mEventPartyChanged.Broadcast(pOldParty, pNewParty);
    mEventFocusedMemberChanged.Broadcast(pOldFocusedMember, mpFocusedMember);
    OnPartyChanged(pOldParty);
}

bool APlayerController_ArenaFight::FindNextFocusableMember(int32 startIndex, bool forwardIteration, int32& focusableMemberIndex)
{
    check(IsValid(mpPossessedParty) && "FindNextFocusableMember() is called while no party is possessed.");

    int32 partySize = mpPossessedParty->GetPartySize();
    int32 increment = forwardIteration ? 1 : -1;
    int32 memberIndex = startIndex;
    int32 countedMembers = 0;
    const TArray<ACreatureBase*>& members = mpPossessedParty->GetMembers();
    bool isFound = false;
    do
    {
        ++countedMembers;
        memberIndex = (memberIndex + partySize + increment) % partySize;
        if (members[memberIndex]->GetDamageable()->IsAlive())
        {
            isFound = true;
            break;
        }
    } while (countedMembers < partySize);
    focusableMemberIndex = isFound ? memberIndex : -1;
    return isFound;
}

void APlayerController_ArenaFight::ValidateFocusedMember()
{
    check(IsValid(mpPossessedParty) && "ValidateFocusedMember() is called while no party is possessed.");

    int32 memberIndex = 0;
    if (mpPossessedParty->IsInParty(mpFocusedMember, memberIndex) == false)
    {
        ResetFocusedMember();
    }
}

bool APlayerController_ArenaFight::Server_NotifyArenaSetup_Validate()
{
    return true;
}

void APlayerController_ArenaFight::Server_NotifyArenaSetup_Implementation()
{
    Cast<AGameMode_ArenaFight>(UGameplayStatics::GetGameMode(this))->OnPlayerArenaSetup(this);
}

bool APlayerController_ArenaFight::Server_SetIsPlanningFinished_Validate(bool isPlanningFinished)
{
    //  ~!~TODO~ Check if now is "planning phase"
    return true;
}

void APlayerController_ArenaFight::Server_SetIsPlanningFinished_Implementation(bool isPlanningFinished)
{
    check(IsValid(mpArenaPlayerState));
    mpArenaPlayerState->mIsPlanningFinished = isPlanningFinished;
    AGameMode_ArenaFight* pArenaGameMode = Cast<AGameMode_ArenaFight>(UGameplayStatics::GetGameMode(this));
    if (IsValid(pArenaGameMode) && pArenaGameMode->IsWaitingForPlayersFinishPlanning())
    {
        if (isPlanningFinished)
        {
            pArenaGameMode->OnPlayerPlanningFinished(this);
        }
        else
        {
            Client_UnlockPlayerControl();
        }
    }

}

bool APlayerController_ArenaFight::Server_NotifyFinishedExecution_Validate()
{
    //  ~!~TODO~ Check if now is "execution phase"
    return mpArenaPlayerState->mIsExecutionFinished == false;
}

void APlayerController_ArenaFight::Server_NotifyFinishedExecution_Implementation()
{
    check(IsValid(mpArenaPlayerState));
    mpArenaPlayerState->mIsExecutionFinished = true;
    AGameMode_ArenaFight* pArenaGameMode = Cast<AGameMode_ArenaFight>(UGameplayStatics::GetGameMode(this));
    if (IsValid(pArenaGameMode))
    {
        pArenaGameMode->OnPlayerExecutionFinished(this);
    }
}

bool APlayerController_ArenaFight::Server_EoARetry_Validate()
{
    return true;
}

void APlayerController_ArenaFight::Server_EoARetry_Implementation()
{
    mpArenaPlayerState->mEoAWantsToRetry = true;
    auto pArenaGameMode = Cast<AGameMode_ArenaFight>(UGameplayStatics::GetGameMode(this));
    if (IsValid(pArenaGameMode))
    {
        pArenaGameMode->OnPlayerEoARetry(this);
    }
}

void APlayerController_ArenaFight::Client_UnlockPlayerControl_Implementation()
{
    UnlockPlayerControl();
}

void APlayerController_ArenaFight::LockPlayerControl()
{
    if (mFocusedAction != nullptr)
    {
        mFocusedAction->DisableSetupVisualization();
    }

    mIsPlayerControlLocked = true;
    K2_LockPlayerControl();
    mEventPlayerControlLocked.Broadcast();
}

void APlayerController_ArenaFight::UnlockPlayerControl()
{
    SetFocusedAction(nullptr);

    K2_UnlockPlayerControl();
    mIsPlayerControlLocked = false;
    mEventPlayerControlUnlocked.Broadcast();
}

void APlayerController_ArenaFight::SetFocusedAction(AAction_Base* newFocusedAction)
{
    if (newFocusedAction == mFocusedAction)
    {
        return;
    }
    else if (newFocusedAction != nullptr)
    {
        if (mpFocusedMember == nullptr)
        {
            UE_LOG(ArenaCommonLog, Warning, TEXT("Can't set focused action as no party member is focused."));
            return;
        }

        UActionManager* actionManager = mpFocusedMember->GetActionManager();
        const TArray<AAction_Base*>& allActions = actionManager->GetAllActions();

        if (allActions.Contains(newFocusedAction) == false)
        {
            UE_LOG(ArenaCommonLog, Warning, TEXT("Can't set focused action as it is not owned by currently focused party member."));
            return;
        }
    }

    AAction_Base* oldFocusedAction = mFocusedAction;
    mFocusedAction = newFocusedAction;

    if (oldFocusedAction != nullptr)
    {
        ResetActionSetup();

        //  Hide setup visualization.
        oldFocusedAction->DisableSetupVisualization();
    }

    if (newFocusedAction != nullptr)
    {
        StartActionSetup();

        //  Show setup visualization.
        newFocusedAction->EnableSetupVisualization();
    }

    K2_OnFocusedActionChanged(oldFocusedAction, newFocusedAction);

    mEventFocusedActionChanged.Broadcast(oldFocusedAction, newFocusedAction);
}

AAction_Base* APlayerController_ArenaFight::GetFocusedAction() const
{
    return mFocusedAction;
}

void APlayerController_ArenaFight::StartActionSetup()
{
    if (mFocusedAction == nullptr)
    {
        UE_LOG(ArenaCommonLog, Warning, TEXT("Failed to start action setup as the focused action is invalid."));
        return;
    }

    check(mpFocusedMember != nullptr && "If focused action is valid, focused member must be valid as well.");


}

void APlayerController_ArenaFight::PushMovementNextStep(const FIntPoint& stepTile)
{
    if (mFocusedAction == nullptr)
    {
        return;
    }
    check(mpFocusedMember != nullptr && "If mFocusedAction != nullptr, mpFocusedMember must be valid.");
    check(mPassMap != nullptr);

    int32 movementRange = mFocusedAction->GetMovementRange();

    UTiledMovement* tileMapMovement = mpFocusedMember->GetTiledMovement();
    FIntPoint partyMemberTile = tileMapMovement->GetCurrentTile();
    
    if (mPassMap->IsTileValid(stepTile) == false)
    {
        UE_LOG(ArenaCommonLog, Warning, TEXT("Can't push movement next step tile. Tile is invalid."));
        return;
    }

    if (stepTile == partyMemberTile)
    {
        mMovementPath.Empty();
        UE_LOG(ArenaCommonLog, Log, TEXT("Tile is already picked. Cutting off path."));
        return;
    }
    
    int32 stepIndex = mMovementPath.Find(stepTile);
    if (stepIndex != INDEX_NONE)
    {
        mMovementPath.SetNum(stepIndex + 1);
        UE_LOG(ArenaCommonLog, Log, TEXT("Tile is already picked. Cutting off path."));
        return;
    }

    if (mMovementPath.Num() >= mFocusedAction->GetMovementRange())
    {
        UE_LOG(ArenaCommonLog, Warning, TEXT("Can't push movement next step tile. Reached max movement range."));
        return;
    }

    bool isTileFree = mPassMap->IsTileFree(stepTile,
                                           MakeBitMask(EOccupationSlot::OccupationSlot_Trap,
                                                       EOccupationSlot::OccupationSlot_StaticLowObstacle,
                                                       EOccupationSlot::OccupationSlot_StaticHighObstacle));
    if (isTileFree == false)
    {
        UE_LOG(ArenaCommonLog, Warning, TEXT("Can't push movement next step tile. Tile is not free."));
        return;
    }

    FIntPoint lastStepTile = mMovementPath.Num() > 0 ? mMovementPath.Last(0) : partyMemberTile;
    bool isTileNeighbor = mPassMap->IsNeighbour(stepTile, lastStepTile);

    if (isTileNeighbor == false)
    {
        UE_LOG(ArenaCommonLog, Warning, TEXT("Can't push movement next step tile. Tile is not neighbor to the last step tile."));
        return;
    }

    mMovementPath.Push(stepTile);
}

void APlayerController_ArenaFight::SetMovementDestination(const FIntPoint& destTile)
{
    if (mFocusedAction == nullptr)
    {
        return;
    }
    check(mpFocusedMember != nullptr && "If mFocusedAction != nullptr, mpFocusedMember must be valid.");
    check(mPassMap != nullptr);

    int32 movementRange = mFocusedAction->GetMovementRange();
    UTiledMovement* tileMapMovement = mpFocusedMember->GetTiledMovement();
    FIntPoint focusedMemberTile = tileMapMovement->GetCurrentTile();

    if (focusedMemberTile != destTile)
    {
        if (mPassMap->IsTileValid(destTile) == false)
        {
            return;
        }
        int32 depthLimit = -1;
        mMovementPath = mPassMap->FindPath(focusedMemberTile,
                                           destTile,
                                           MakeBitMask(EOccupationSlot::OccupationSlot_StaticLowObstacle,
                                                       EOccupationSlot::OccupationSlot_StaticHighObstacle),
                                           MakeBitMask(EOccupationSlot::OccupationSlot_Trap),
                                           depthLimit,
                                           movementRange,
                                           true);
    }
    else
    {
        mMovementPath.Empty();
    }
}

void APlayerController_ArenaFight::ResetActionSetup()
{
    mMovementPath.Empty();
    mExecutionDirection = 0;
}

void APlayerController_ArenaFight::FinishActionSetup()
{
    if (mFocusedAction == nullptr)
    {
        return;
    }
    check(mpFocusedMember != nullptr && "If mFocusedAction != nullptr, mpFocusedMember must be valid.");

    UActionManager* actionManager = mpFocusedMember->GetActionManager();

    mFocusedAction->SetMovementPath(mMovementPath);
    mFocusedAction->SetExecutionDirection(mExecutionDirection);
    if (mFocusedAction->Setup())
    {
        actionManager->SetActionToExecute(mFocusedAction);
        mEventActionToExecuteSetup.Broadcast(mFocusedAction);
    }
    else
    {
        actionManager->ResetActionToExecute();
        mEventActionToExecuteSetup.Broadcast(nullptr);
    }
}

void APlayerController_ArenaFight::OnActionSetupPickMovementStepTile()
{
    if (mPassMap == nullptr || 
        mpFocusedMember == nullptr ||
        mFocusedAction == nullptr)
    {
        return;
    }

    PushMovementNextStep(mTileUnderCursor);
}

void APlayerController_ArenaFight::OnActionSetupPickMovementDestinationTile()
{
    if (mPassMap == nullptr || 
        mpFocusedMember == nullptr ||
        mFocusedAction == nullptr)
    {
        return;
    }

    SetMovementDestination(mTileUnderCursor);
}

void APlayerController_ArenaFight::OnActionSetupPickExecutionDirection()
{
    if (mFlipFlopRayMarch)
    {
        mRayMarchFirstTile = mTileUnderCursor;
    }
    else
    {
        mRayMarchLastTile = mTileUnderCursor;
        FIntPoint blockingTile(-1, -1);
        bool isDirectlyVisible = mPassMap->DirectVisibility(mRayMarchFirstTile,
                                                            mRayMarchLastTile,
                                                            MakeBitMask(EOccupationSlot::OccupationSlot_StaticLowObstacle, EOccupationSlot::OccupationSlot_StaticHighObstacle),
                                                            blockingTile);
        UE_LOG(ArenaCommonLog, Log, TEXT("Direct visibility from [%i, %i] to [%i, %i] is %s. Blocking tile is [%i; %i]."),
               mRayMarchFirstTile.X,
               mRayMarchFirstTile.Y,
               mRayMarchLastTile.X,
               mRayMarchLastTile.Y,
               isDirectlyVisible ? TEXT("NOT BLOCKED") : TEXT("BLOCKED"),
               blockingTile.X,
               blockingTile.Y);
    }
    mFlipFlopRayMarch = !mFlipFlopRayMarch;


    if (mPassMap == nullptr ||
        mpFocusedMember == nullptr ||
        mFocusedAction == nullptr)
    {
        return;
    }

    FIntPoint focusedMemberTile = mpFocusedMember->GetTiledMovement()->GetCurrentTile();
    FIntPoint lastStepTile = mMovementPath.Num() > 0 ? mMovementPath.Last(0) : focusedMemberTile;
    mExecutionDirection = mPassMap->GetDirectionBetweenTiles(lastStepTile,
                                                             mTileUnderCursor);
    UE_LOG(ArenaCommonLog, Log, TEXT("New execution direction for action: %i"),
           mExecutionDirection);

    FinishActionSetup();
}

void APlayerController_ArenaFight::OnShowAllPartyMembersActionsSetup()
{
    const TArray<ACreatureBase*>& partyMembers = mpPossessedParty->GetMembers();
    for (ACreatureBase* partyMember : partyMembers)
    {
        UActionManager* actionManager = partyMember->GetActionManager();
        check(actionManager != nullptr);
        AAction_Base* setupAction = actionManager->GetActionToExecute();
        if (setupAction != nullptr)
        {
            setupAction->EnableSetupVisualization();
        }
    }
}

void APlayerController_ArenaFight::OnHideAllPartyMembersActionsSetup()
{
    const TArray<ACreatureBase*>& partyMembers = mpPossessedParty->GetMembers();
    for (ACreatureBase* partyMember : partyMembers)
    {
        UActionManager* actionManager = partyMember->GetActionManager();
        check(actionManager != nullptr);
        AAction_Base* setupAction = actionManager->GetActionToExecute();
        if (setupAction != nullptr &&
            setupAction != mFocusedAction)
        {
            setupAction->DisableSetupVisualization();
        }
    }
}

void APlayerController_ArenaFight::UpdateTileUnderCursor()
{
    if (mPassMap == nullptr)
    {
        return;
    }

    ULocalPlayer* localPlayer = Cast<ULocalPlayer>(Player);
    if (localPlayer == nullptr ||
        localPlayer->ViewportClient == nullptr)
    {
        return;
    }

    FVector2D mousePosition;
    if (localPlayer->ViewportClient->GetMousePosition(mousePosition))
    {
        FVector cameraLocation;
        FVector cursorDirection;
        if (UGameplayStatics::DeprojectScreenToWorld(this,
                                                     mousePosition,
                                                     cameraLocation,
                                                     cursorDirection))
        {
            mPassMap->LineTraceTile(cameraLocation,
                                    cameraLocation + cursorDirection * HitResultTraceDistance,
                                    mTileUnderCursor);
        }
    }
}
