

#include "Arena/ActionMap_MixedConflicts.h"
#include "Arena/BattleLog/BattleLog.h"
#include "Arena/BattleLog/BattleLogItem_ActionCancellation.h"
#include "BitmaskOperator.h"


void AActionMap_MixedConflicts::SubmitAction_Inner(AAction_Base* action,
                                                   int32 executionPriority)
{
    //	~!~ ~UGLY~ Here should be better solution for registering actions for a multiple
    //	owned tiles. The current implementation demands all owned tiles to be ordered in
    //	an array and then each tile gets decreased weight by its' index in this array.
    const TArray<FActionDistributionTile>& distribution = action->GetDistribution();
    check(distribution.Num() > 0 && "There is no distribution tiles for registered action.");

    ActionEntry newAtionEntry = {};
    newAtionEntry.mpAction = action;
    newAtionEntry.mOrder = 0;
    for (const auto& distributionEntry : distribution)
    {
        newAtionEntry.mTile = distributionEntry.mTile;
        newAtionEntry.mEntryBehavior = distributionEntry.mEntryBehavior;
        newAtionEntry.mPriority = executionPriority + mPriorityShiftAboveUnitValue;
        ++newAtionEntry.mOrder;
        mSubmittedActionsEntries.Push(newAtionEntry);

        //  Move priority shift window:
        ++mPriorityShiftAboveUnitValue;
        checkSlow(mPriorityShiftAboveUnitValue < UNKNOWN_PRIORITY_UNIT_VALUE);
    }
}

void AActionMap_MixedConflicts::ResolveActionConflicts_Inner()
{
    FActionCollisionResolveContext context;

    //	Firstly sort all of prioritized actions by their priority from highest to lowest:
    auto CompareActionsByPriority = [] (const ActionEntry& leftActionEntry,
                                        const ActionEntry& rightActionEntry)
    {
        return leftActionEntry.mPriority > rightActionEntry.mPriority ||
            (leftActionEntry.mPriority == rightActionEntry.mPriority &&
             leftActionEntry.mOrder > rightActionEntry.mOrder);
    };
    mSubmittedActionsEntries.Sort(CompareActionsByPriority);

    //	Map of interrupted actions which can be used to determine if an action
    //	was previously interrupted during the collision resolve. Additionaly it
    //	stores a priority at which the action was interrupted.
    TMap<AAction_Base*, int32> actionsStatus;

    auto MarkActionCanceled = [&actionsStatus] (const ActionEntry& actionEntry)
    {
        actionsStatus.FindOrAdd(actionEntry.mpAction) = 0;
    };

    auto MarkActionInterrupted = [&actionsStatus] (const ActionEntry& actionEntry,
                                                   bool includingEntry)
    {
        actionsStatus.FindOrAdd(actionEntry.mpAction) = actionEntry.mOrder + includingEntry ? 0 : 1;
    };

    auto IsActionEntryInterrupted = [&actionsStatus] (const ActionEntry& actionEntry)
    {
        int32* pInterruptOrder = actionsStatus.Find(actionEntry.mpAction);
        int32 interruptOrder = (pInterruptOrder == nullptr ? INT32_MAX : *pInterruptOrder);
        return interruptOrder <= actionEntry.mOrder;
    };

    //	Each action should resolve collisions with the lower priority actions in
    //	the stack of its' placement:
    for (int32 h = 0; mSubmittedActionsEntries.IsValidIndex(h); ++h)
    {
        ActionEntry& highPrioActionEntry = mSubmittedActionsEntries[h];
        //	If an action is already interrupted it doesn't participate in collisions:
        if (IsActionEntryInterrupted(highPrioActionEntry) == false)
        {
            if (highPrioActionEntry.mpAction->IsCollisionResolveStarted() == false)
            {
                highPrioActionEntry.mpAction->StartCollisionResolve(context);
            }

            //	Iterate over all actions with the lower priority and solve collisions only for actions
            //	at the same tile with the high priority action:
            for (int32 l = h + 1; mSubmittedActionsEntries.IsValidIndex(l); ++l)
            {
                const ActionEntry& lowPrioActionEntry = mSubmittedActionsEntries[l];
                if (lowPrioActionEntry.mpAction != highPrioActionEntry.mpAction &&
                    lowPrioActionEntry.mTile == highPrioActionEntry.mTile &&
                    IsActionEntryInterrupted(lowPrioActionEntry) == false)
                {
                    const FIntPoint& collisionTile = lowPrioActionEntry.mTile;
                    AAction_Base* pHPAction = highPrioActionEntry.mpAction;
                    AAction_Base* pLPAction = lowPrioActionEntry.mpAction;

                    //  Resolve movement:
                    bool isPretendingOnTileHP = pHPAction->IsPretendingOnTile(collisionTile);
                    bool isPretendingOnTileLP = pLPAction->IsPretendingOnTile(collisionTile);
                    bool isCollidingExecutorTile = pLPAction->GetExecutorPlacement() == collisionTile;

                    auto resolveResult = pHPAction->ResolveCollision(context,
                                                                     pLPAction,
                                                                     collisionTile,
                                                                     highPrioActionEntry.mEntryBehavior,
                                                                     lowPrioActionEntry.mEntryBehavior);
                    //  ~!~ Idle actions have UNKNOWN_IDLE_ACTION_BEHAVIOR usage. Idle actions can't be canceled:
                    if (pLPAction->GetBehavior() != UNKNOWN_IDLE_ACTION_BEHAVIOR)
                    {
                        if (IsFlagAtBitMask(lowPrioActionEntry.mEntryBehavior, EActionBehavior::ActionBehavior_Executor) &&
                            pLPAction->IncreaseCancellationCounter(context,
                                                                   resolveResult.mCancellationCounterChange,
                                                                   pHPAction))
                        {
                            //	If action is fully canceled - set it's interruption order to the maximum.
                            MarkActionCanceled(lowPrioActionEntry);
                        }
                        else if (resolveResult.mInterrupt)
                        {
                            if (pLPAction->Interrupt(context,
                                                     collisionTile,
                                                     pHPAction))
                            {
                                //  ~BATTLELOG~
                                auto pActionCancellation = NewObject<UBattleLogItem_ActionCancellation>();
                                pActionCancellation->mCancelledActionInfo.SetFromAction(pLPAction);
                                //  ~TODO~ Set pActionCancellation->mCancelledByActionsInfo
                                //  ~?~ May store all cancellation (and mutation) events and push it in the end of collision resolve.
                                GetBattleLog()->PushBattleLogItem(pActionCancellation);

                                MarkActionCanceled(lowPrioActionEntry);
                            }
                            else if (pLPAction->IsLowOrderInterruptionEnabled())
                            {
                                MarkActionInterrupted(lowPrioActionEntry,
                                                      resolveResult.mIncludingCollisionTile);
                            }
                        }
                    }
                }
            }
        }
    }

    for (auto* pAction : mSubmittedActions)
    {
        pAction->FinishCollisionResolve();
    }
}

void AActionMap_MixedConflicts::OnPrepareForPlanning()
{
    Super::OnPrepareForPlanning();

    mSubmittedActionsEntries.Empty();
    mPriorityShiftAboveUnitValue = 0;
}
