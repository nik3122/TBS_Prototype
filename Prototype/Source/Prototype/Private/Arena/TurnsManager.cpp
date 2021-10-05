// Fill out your copyright notice in the Description page of Project Settings.

#include "Arena/TurnsManager.h"
#include "Arena/ActionMap.h"
#include "Arena/ArenaManager.h"
#include "Arena/ArenaUtilities.h"
#include "Arena/BattleLog/BattleLog.h"
#include "Arena/BattleLog/BattleLogItem_NewTurn.h"
#include "Arena/GameState_Arena.h"


ATurnsManager::ATurnsManager(const FObjectInitializer& objInit)
    : Super(objInit)
{
    PrimaryActorTick.bCanEverTick = true;

    SetCanBeDamaged(false);
    bCollideWhenPlacing = false;
    SetHidden(true);
    bReplicates = true;
    bAlwaysRelevant = true;
}

void ATurnsManager::SetupSystem(AActionMap* pActionMap)
{
    //
}

void ATurnsManager::QueryTurnExecution(UObject* pTurnExecutor)
{
    UE_LOG(ArenaCommonLog, Log, TEXT("QUERY TURN #%i EXECUTION: %s"), mCurrentTurn, *GetTurnExecutorName(pTurnExecutor));
    mTurnExecutors.Add(pTurnExecutor);
}

void ATurnsManager::ReleaseTurnExecution(UObject* pTurnExecutor)
{
    FString turnExecutorName = GetTurnExecutorName(pTurnExecutor);
    if (mTurnExecutors.Num() > 0)
    {
        int32 removedItemsCount = mTurnExecutors.Remove(pTurnExecutor);
        if (removedItemsCount == 0)
        {
            UE_LOG(ArenaCommonLog, Error, TEXT("%s trying to RELEASE TURN #%i EXECUTION while not registered for turn execution."), *turnExecutorName, mCurrentTurn);
        }
        else
        {
            UE_LOG(ArenaCommonLog, Log, TEXT("RELEASE TURN #%i EXECUTION: %s"), mCurrentTurn, *turnExecutorName);
        }
    }
    else
    {
        UE_LOG(ArenaCommonLog, Error, TEXT("%s trying to RELEASE TURN #%i EXECUTION while no turn executors are present."), *turnExecutorName, mCurrentTurn);
    }
}

void ATurnsManager::StartPlanning()
{
    check(mIsExecuting == false);
    Multicast_StartPlanning();
}

void ATurnsManager::FinishPlanning()
{
    check(mIsExecuting == false);
    Multicast_FinishPlanning();
}

void ATurnsManager::ExecuteTurn()
{
    check(mIsExecuting == false);

    mEventStoreTurnStateResolved.ExecuteIfBound();

    UBattleLog* pBattleLog = UArenaUtilities::GetBattleLog(this);
    pBattleLog->PushBattleLogItem(pBattleLog->CreateBattleLogItem_NewTurn(mCurrentTurn));

    mEventPreExecution.Broadcast();

    mEventApplyStatusEffectsBeforeActions.Broadcast();

    mEventApplyActionsPreExecution.ExecuteIfBound();

    mIsExecuting = true;
    if (mSkipExecution == false)
    {
        mEventStartExecution.Broadcast();

        UE_LOG(ArenaCommonLog, Log, TEXT("START TURN #%i EXECUTION with %i turn executors."), mCurrentTurn, mTurnExecutors.Num());

        mExecutionDuration = 0.0f;
    }
    else
    {
        UE_LOG(ArenaCommonLog, Log, TEXT("SKIP TURN #%i EXECUTION"), mCurrentTurn);
        StopTurnExecution();
    }
}

void ATurnsManager::InterruptExecution()
{
    TryStopTurnExecution(true);
}

int32 ATurnsManager::GetCurrentTurn() const
{
    return mCurrentTurn;
}

int32 ATurnsManager::GetTurnsLimit() const
{
    return mTurnsLimit;
}

float ATurnsManager::GetLastTurnDuration() const
{
    return mExecutionDuration;
}

bool ATurnsManager::IsExecuting() const
{
    return mIsExecuting;
}

void ATurnsManager::BeginPlay()
{
    Super::BeginPlay();

    UArenaUtilities::GetArenaManager(this)->RegisterSystem(this);
}

void ATurnsManager::Tick(float deltaTime)
{
    Super::Tick(deltaTime);
    if (mIsExecuting)
    {
        mExecutionDuration += deltaTime;
        TryStopTurnExecution();
    }
}

void ATurnsManager::TryStopTurnExecution(bool forceEndTurn)
{
    check(mIsExecuting);
    bool canEndTurn = (mTurnExecutors.Num() == 0) || forceEndTurn;

#if !UE_BUILD_SHIPPING
    if (mAutoTurnEndOvertime <= mExecutionDuration)
    {
        for (auto* pTurnMaker : mTurnExecutors)
        {
            UE_LOG(ArenaCommonLog, Warning, TEXT("%s reached OVERTIME (animation hanging, long execution, etc.)."), *pTurnMaker->GetName());
        }
        canEndTurn = true;
        UE_LOG(ArenaCommonLog, Warning, TEXT("Terminate turn! This can cause bugs."));
    }
#endif // !UE_BUILD_SHIPPING

    if (canEndTurn)
    {
        StopTurnExecution();
    }
}

void ATurnsManager::StopTurnExecution()
{
    check(mIsExecuting);

    mIsExecuting = false;

    if (mSkipExecution == false)
    {
        mEventStopExecution.Broadcast();
    }

    UE_LOG(ArenaCommonLog, Log, TEXT("STOP TURN #%i EXECUTION"), mCurrentTurn);

    mEventPostExecution.Broadcast();

    mEventApplyStatusEffectsAfterActions.Broadcast();

    mEventUpdateDamageable.Broadcast();

    mEventFinalizeTurn.Broadcast();

    mEventStoreTurnStateFinal.ExecuteIfBound();

    mTurnExecutors.Reset();
    ++mCurrentTurn;

    //  This always must be valid: either GameMode or PlayerController must be subscribed:
    mEventTurnFinished.Execute();
}

void ATurnsManager::SetSkipExecution(bool skipExecution)
{
    check(mIsExecuting == false);
    mSkipExecution = skipExecution;
}

void ATurnsManager::Multicast_StartPlanning_Implementation()
{
    UE_LOG(ArenaCommonLog, Log, TEXT("START TURN #%i PLANNING"), mCurrentTurn);

    mEventPrepareForPlanning.Broadcast();

    mEventStoreTurnStateInitial.ExecuteIfBound();

    mEventUnlockPlayerControl.ExecuteIfBound();
}

void ATurnsManager::Multicast_FinishPlanning_Implementation()
{
    UE_LOG(ArenaCommonLog, Log, TEXT("FINISH TURN #%i PLANNING"), mCurrentTurn);
}

FString ATurnsManager::GetTurnExecutorName(UObject* pTurnExecutor) const
{
    AAction_Base* pAction = Cast<AAction_Base>(pTurnExecutor);
    return IsValid(pAction) ? pAction->GetExecutorActor()->GetName() + " @ " + pAction->GetName() : pTurnExecutor->GetName();
}
