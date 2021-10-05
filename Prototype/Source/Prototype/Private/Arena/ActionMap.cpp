// Fill out your copyright notice in the Description page of Project Settings.

#include "Arena/ActionMap.h"
#include "Arena/ActionManager.h"
#include "Arena/ActionCollisionResolveContext.h"
#include "Arena/ArenaManager.h"
#include "Arena/ArenaManager_Interface.h"
#include "Arena/ArenaUnit_Interface.h"
#include "Arena/ArenaUtilities.h"
#include "Arena/BattleLog/BattleLog.h"
#include "Arena/GameState_Arena.h"
#include "Arena/TurnsManager.h"
#include "BitmaskOperator.h"
#include "UnknownCommon.h"


AActionMap::AActionMap()
    : mIsResolvingCollisions(false)
{
    PrimaryActorTick.bCanEverTick = true;

    mBattleLog = CreateDefaultSubobject<UBattleLog>("BattleLog");
}

void AActionMap::SetupSystem(ATurnsManager* pTurnsManager)
{
    pTurnsManager->mEventPrepareForPlanning.AddDynamic(this, &AActionMap::OnPrepareForPlanning);
    pTurnsManager->mEventApplyActionsPreExecution.BindUObject(this, &AActionMap::OnApplyActionsPreExecution);
    pTurnsManager->mEventStartExecution.AddDynamic(this, &AActionMap::OnStartExecution);
    pTurnsManager->mEventStopExecution.AddDynamic(this, &AActionMap::OnStopExecution);
    pTurnsManager->mEventPostExecution.AddDynamic(this, &AActionMap::OnPostExecution);
}

void AActionMap::RegisterActionManager(UActionManager* pActionManager)
{
    check(IsValid(pActionManager) && "ActionManager shouldn't be NULL.");
    mRegisteredActionManagers.Add(pActionManager);

    //  ~!~?~ Before adding 
    //  Update expected submitting action managers if it is in the planning phase:
    mExpectedSubmittingActionManagers.Add(pActionManager);
}

void AActionMap::UnregisterActionManager(UActionManager* pActionManager)
{
    check(IsValid(pActionManager) && "ActionManager shouldn't be NULL.");
    int32 removedItemsCount = mRegisteredActionManagers.RemoveSingleSwap(pActionManager, false);
    //check(removedItemsCount != 0 && "ActionManager wasn't registered.");
}

void AActionMap::StartActionSubmitting()
{
    mIsActionSubmittingAllowed = true;
    for (UActionManager* pActionManager : mRegisteredActionManagers)
    {
        if (pActionManager->IsActive())
        {
            pActionManager->SyncAndSubmitActionToExecute();
        }
    }
}

bool AActionMap::CanSubmitAction() const
{
    return mIsActionSubmittingAllowed;
}

void AActionMap::SubmitAction(UActionManager* actionManager,
                              AAction_Base* action,
                              int32& outExecutionPriority)
{
    check(HasAuthority() == false || mIsActionSubmittingAllowed && "Submittion is currently not allowed.");
    check(mIsResolvingCollisions == false && "Try to register action while resolving collisions.");
    check(IsValid(action) && "Try to register invalid action.");

    if (mExpectedSubmittingActionManagers.Remove(actionManager) == 0)
    {
        UE_LOG(ArenaCommonLog, Error, TEXT("Action manager of %s is not expected to submit an action."),
               *actionManager->GetOwner()->GetName());
        return;
    }

    outExecutionPriority = CalculateActionPriority(action);
    action->SetExecutionPriority(outExecutionPriority);
    SubmitAction_Inner(action, outExecutionPriority);

    mSubmittedActions.Push(action);

    //  Wait untill all registered action managers will submit actions:
    if (mExpectedSubmittingActionManagers.Num() == 0)
    {
        FinishActionSubmitting();
    }
}

void AActionMap::ResolveActionConflicts()
{
    mIsResolvingCollisions = true;

    ResolveActionConflicts_Inner();

    mIsResolvingCollisions = false;
}

const TArray<AAction_Base*>& AActionMap::GetSubmittedActions() const
{
    return mSubmittedActions;
}

UBattleLog* AActionMap::GetBattleLog() const
{
    return mBattleLog;
}

void AActionMap::BeginPlay()
{
    Super::BeginPlay();

    UArenaUtilities::GetArenaManager(this)->RegisterSystem(this);
}

int32 AActionMap::CalculateActionPriority(AAction_Base* pAction)
{
    auto executorArenaUnitInterface = pAction->GetExecutorArenaUnit();
    UActionManager* pExecutorActionManager = IArenaUnit_Interface::Execute_GetActionManager(executorArenaUnitInterface.GetObject());
    int32 reactionRate = pExecutorActionManager->GetCurrentReactionRate();
    int32 actionSpeed = pAction->GetInitiative();
    int32 actionPriority = (reactionRate + actionSpeed) * UNKNOWN_PRIORITY_UNIT_VALUE;
    return actionPriority;
}

void AActionMap::SubmitAction_Inner(AAction_Base* action,
                                    int32 executionPriority)
{
    UE_LOG(ArenaCommonLog, Error, TEXT("%s::SubmitAction_Inner() is not implemented."),
           *GetClass()->GetName());
}

void AActionMap::ResolveActionConflicts_Inner()
{
    UE_LOG(ArenaCommonLog, Error, TEXT("%s::ResolveActionConflicts_Inner() is not implemented."),
           *GetClass()->GetName());
}

void AActionMap::OnPrepareForPlanning()
{
    //  ~BATTLELOG~
    //  ~?~ Temporary disabled as the log items are generated and PUSHED in the Apply functions.
    /*TArray<UBattleLogItem_Base*> battleLogItems;
    for (AAction_Base* pAction : mSubmittedActions)
    {
        battleLogItems.Append(pAction->GenerateBattleLogItemsForCurrentTurn());
    }
    if (battleLogItems.Num() > 0)
    {
        GetBattleLog()->PushBattleLogItems(battleLogItems);
    }*/

    mSubmittedActions.Empty();

    mExpectedSubmittingActionManagers.Empty(mRegisteredActionManagers.Num());
    for (UActionManager* actionManager : mRegisteredActionManagers)
    {
        if (actionManager->IsActive())
        {
            mExpectedSubmittingActionManagers.Add(actionManager);
        }
    }
}

void AActionMap::OnApplyActionsPreExecution()
{
    //  Apply submitted actions in order from HP to LP:
    for (AAction_Base* pAction : mSubmittedActions)
    {
        UArenaUtilities::GetTurnsManager(this)->QueryTurnExecution(pAction);
        if (pAction->IsCanceled() == false)
        {
            pAction->PreExecutionApply();
        }
    }
}

void AActionMap::OnStartExecution()
{
    for (AAction_Base* pAction : mSubmittedActions)
    {
        pAction->mEventActionExecuted.BindUObject(this, &AActionMap::OnActionExecuted);
        pAction->StartExecution();
    }
}

void AActionMap::OnStopExecution()
{
    for (AAction_Base* pAction : mSubmittedActions)
    {
        //  ~?~ StopExecution will fire AAction_Base::mEventActionExecuted
        //pAction->mEventActionExecuted.Unbind();
        pAction->StopExecution();
    }
}

void AActionMap::OnPostExecution()
{
    //  Apply submitted actions in order from HP to LP:
    for (AAction_Base* pAction : mSubmittedActions)
    {
        if (pAction->IsCanceled() == false)
        {
            pAction->PostExecutionApply();
        }
    }
}

void AActionMap::OnActionExecuted(AAction_Base* pExecutedAction)
{
    pExecutedAction->mEventActionExecuted.Unbind();
    UArenaUtilities::GetTurnsManager(this)->ReleaseTurnExecution(pExecutedAction);
}

void AActionMap::FinishActionSubmitting()
{
    mSubmittedActions.Sort([](const AAction_Base& left, const AAction_Base& right) {
        return left.GetExecutionPriority() == right.GetExecutionPriority() ? left.GetActionID() < right.GetActionID() :
            left.GetExecutionPriority() > right.GetExecutionPriority();
    });

    //  ~?~ Next tick is needed to avoid ActionManager submitting an action in callstack:
    GetWorld()->GetTimerManager().SetTimerForNextTick([this]() {
        mIsActionSubmittingAllowed = false;
        mEventActionsSubmittingFinished.Execute();
    });
}