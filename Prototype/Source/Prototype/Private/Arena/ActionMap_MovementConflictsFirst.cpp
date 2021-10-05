

#include "Arena/ActionMap_MovementConflictsFirst.h"
#include "Arena/Action_SeparateMovement_Base.h"
#include "Arena/ArenaSettings.h"
#include "Arena/ArenaUtilities.h"
#include "Arena/BattleLog/BattleLog.h"
#include "Arena/BattleLog/BattleLogItem_CreatureMovement.h"
#include "Arena/BattleLog/BattleLogItem_PushCreature.h"
#include "Arena/BattleLog/BattleLogItem_RamCreature.h"
#include "Arena/Damageable.h"
#include "Arena/OccupationSlot.h"
#include "Arena/PassMap.h"
#include "Arena/TiledMovement.h"
#include "Arena/TurnsManager.h"
#include "Containers/Map.h"


void AActionMap_MovementConflictsFirst::BeginPlay()
{
    Super::BeginPlay();

    UArenaSettings* arenaSettings = UArenaSettings::Get();
    mPushDamage = arenaSettings->mMovementConflictsPushDamage;
    mRamDamage = arenaSettings->mMovementConflictsRamDamage;
}

void AActionMap_MovementConflictsFirst::Tick(float deltaSconds)
{
    Super::Tick(deltaSconds);

    if (mActorToPush != nullptr)
    {
        mPushElapsedTime += deltaSconds;
        float lerpFactor = FMath::Min(mPushElapsedTime / mPushDuration, 1.f);
        FVector newLocation = FMath::Lerp(mPushStartLocation, mPushEndLocation, lerpFactor);
        mActorToPush->SetActorLocation(newLocation);
        if (lerpFactor >= 1.f)
        {
            OnPushFinished();
        }
    }
}

void AActionMap_MovementConflictsFirst::SetupSystem(ATurnsManager* turnsManager)
{
    Super::SetupSystem(turnsManager);

    mPassMap = UArenaUtilities::GetPassMap(this);
}

int32 AActionMap_MovementConflictsFirst::CalculateActionPriority(AAction_Base* action)
{
    return action->GetInitiative();
}

void AActionMap_MovementConflictsFirst::SubmitAction_Inner(AAction_Base* action,
                                                           int32 executionPriority)
{
    //
}

void AActionMap_MovementConflictsFirst::ResolveActionConflicts_Inner()
{
    //  Actions are sorted in descending order of their priorities.
    //  First resolve movement conflicts.

    mMovementEvents.SetNum(0, false);

    TMap<AAction_Base*, FIntPoint> finalTiles;
    finalTiles.Reserve(mSubmittedActions.Num());
    //ResolveMovementConflicts_IgnoreIntermediateObstacles(finalTiles);
    ResolveMovementConflicts_StopAtClosestObstacle(finalTiles);

    APassMap* passMap = UArenaUtilities::GetPassMap(this);
    passMap->ResetAllOccupationsAtSlot(EOccupationSlot::OccupationSlot_ArenaUnit, true);

    //  Occupy final tiles:
    for (TPair<AAction_Base*, FIntPoint>& pairActionTile : finalTiles)
    {
        AAction_Base* action = pairActionTile.Key;
        FIntPoint tile = pairActionTile.Value;
        action->SetFinalTile(tile);
        passMap->OccupyTile(tile,
                            EOccupationSlot::OccupationSlot_ArenaUnit,
                            action->GetExecutorActor());
    }
}

void AActionMap_MovementConflictsFirst::OnApplyActionsPreExecution()
{
    /*for (AAction_Base* submittedAction : mSubmittedActions)
    {
        AAction_SeparateMovement_Base* action = Cast<AAction_SeparateMovement_Base>(submittedAction);
        if (action != nullptr)
        {
            UDamageable* damageable = IArenaUnit_Interface::Execute_GetDamageable(action->GetExecutorActor());
            if (damageable == nullptr ||
                damageable->IsAlive())
            {
                action->Apply();
            }
        }
        else
        {
            UE_LOG(ArenaCommonLog, Warning, TEXT("Action %s can't be applied. It doesn't inherit AAction_SeparateMovement_Base."),
                   *submittedAction->GetName());
        }
    }*/
}

void AActionMap_MovementConflictsFirst::OnStartExecution()
{
    //  ~!~R~ Old pipeline style:
    UArenaUtilities::GetTurnsManager(this)->QueryTurnExecution(this);

    APassMap* passMap = UArenaUtilities::GetPassMap(this);

    //  ~!~ If movement animations are skiped, first movement event execution may trigger execution of conflict events at once.
    mCountOfExecutingNonConflictMoveEvents = 0;

    //  Start executing non-conflict movements.
    for (const FMovementEvent& movementEvent : mMovementEvents)
    {
        if (movementEvent.mType == EMovementEventType::Move &&
            movementEvent.mHasConflict == false)
        {
            ++mCountOfExecutingNonConflictMoveEvents;
            UTiledMovement::FDelegateMovementStarted delegate;
            delegate.BindUObject(this, &AActionMap_MovementConflictsFirst::OnNonConflictMovementFinished);
            ExecuteMoveEvent(movementEvent, delegate);
        }
    }

    if (mCountOfExecutingNonConflictMoveEvents == 0)
    {
        StartExecutingConflictMovementEvents();
    }
}

void AActionMap_MovementConflictsFirst::OnStopExecution()
{

}

void AActionMap_MovementConflictsFirst::OnPostExecution()
{

}

void AActionMap_MovementConflictsFirst::ResolveMovementConflicts_IgnoreIntermediateObstacles(TMap<AAction_Base*, FIntPoint>& outFinalTiles)
{
    APassMap* passMap = UArenaUtilities::GetPassMap(this);
    UBattleLog* battleLog = UArenaUtilities::GetBattleLog(this);

    TMap<FIntPoint, AAction_Base*> claimedTiles;
    TSet<AAction_Base*> conflictActions;

    //  Actions which are "pushed" should not perform movement.
    //  ~?~ conflictActions may be used if both push and ram cancells movement.
    TSet<AAction_Base*> cancelledMovementActions;

    //  Initially all actions claims their executors' placements.
    for (AAction_Base* action : mSubmittedActions)
    {
        claimedTiles.Add(action->GetExecutorPlacement(), action);
    }

    for (AAction_Base* action : mSubmittedActions)
    {
        const TArray<FIntPoint>& movementPath = action->GetActionSetupData().mMovementTiles;
        if (movementPath.Num() == 0)
        {
            continue;
        }

        if (cancelledMovementActions.Contains(action) == false)
        {
            continue;
        }

        FMovementEvent actionMovementEvent;
        actionMovementEvent.mAction = action;

        FMovementEvent extraMovementEvent;
        bool useExtraMovementEvent = false;

        FIntPoint tileToClaim = action->GetExecutorPlacement();

        //  If action is moving remove initially claimed tile to allow pushing other actions to that tile:
        claimedTiles.Remove(tileToClaim);

        for (int32 stepIndex = movementPath.Num() - 1; stepIndex >= 0; --stepIndex)
        {
            FIntPoint stepTile = movementPath[stepIndex];

            //  Action which already claimed step tile.
            AAction_Base** claimantAction = claimedTiles.Find(stepTile);

            if (claimantAction == nullptr)
            {
                actionMovementEvent.mType = EMovementEventType::Move;
                actionMovementEvent.mArenaUnit = action->GetExecutorArenaUnit();
                actionMovementEvent.mPath.Append(movementPath.GetData(), stepIndex + 1);
                useExtraMovementEvent = false;

                tileToClaim = stepTile;
                break;
            }
            else
            {
                AAction_Base* conflictAction = *claimantAction;
                FIntPoint prevStepTile = stepIndex > 0 ? movementPath[stepIndex - 1] : action->GetExecutorPlacement();
                int32 pushDirection = passMap->GetNeighbourDirection(stepTile, prevStepTile);
                FIntPoint pushTile = passMap->GetNeighbour(stepTile, pushDirection);

                bool isPushTileNotClaimed = claimedTiles.Find(pushTile) == nullptr;
                if (isPushTileNotClaimed)
                {
                    bool isPushTileFree = passMap->IsTileFree_Safe(pushTile,
                                                                   MakeBitMask(EOccupationSlot::OccupationSlot_StaticLowObstacle,
                                                                               EOccupationSlot::OccupationSlot_StaticHighObstacle));

                    if (isPushTileFree)
                    {
                        actionMovementEvent.mType = EMovementEventType::MoveAndPush;
                        actionMovementEvent.mArenaUnit = action->GetExecutorArenaUnit();
                        actionMovementEvent.mPath.Append(movementPath.GetData(), stepIndex + 1);

                        useExtraMovementEvent = true;
                        extraMovementEvent.mType = EMovementEventType::Push;
                        extraMovementEvent.mAction = conflictAction;
                        extraMovementEvent.mArenaUnit = conflictAction->GetExecutorArenaUnit();
                        extraMovementEvent.mPath.Push(pushTile);

                        //  Push claiming action to the opposite neighbour tile.
                        claimedTiles.FindOrAdd(pushTile) = conflictAction;

                        conflictActions.Add(action);
                        conflictActions.Add(conflictAction);

                        cancelledMovementActions.Add(conflictAction);

                        //  Step tile can now be claimed by the given action.
                        tileToClaim = stepTile;

                        //  Affect creature:
                        {
                            UBattleLogItem_PushCreature* battleLogItem = nullptr;
                            auto pusherCreature = Cast<ACreatureBase>(action->GetExecutorArenaUnit().GetObject());
                            auto targetCreature = Cast<ACreatureBase>(conflictAction->GetExecutorArenaUnit().GetObject());
                            if (pusherCreature != nullptr &&
                                targetCreature != nullptr)
                            {
                                FIntPoint fromTile = targetCreature->GetTiledMovement()->GetCurrentTile();
                                FIntPoint toTile = pushTile;
                                battleLogItem = battleLog->CreateBattleLogItem_PushCreature(pusherCreature,
                                                                                            targetCreature,
                                                                                            fromTile,
                                                                                            toTile,
                                                                                            FDamageParameters());
                            }
                            actionMovementEvent.mBattleLogItem = battleLogItem;

                            bool isVictimKilled = false;
                            AffectPushVictim(action->GetExecutorActor(),
                                             conflictAction->GetExecutorActor(),
                                             battleLogItem,
                                             isVictimKilled);
                        }

                        break;
                    }
                }

                //  If claimant action can't be pushed, check if it can be "rammed", stepping into the previous tile.
                bool isPrevStepTileNotClaimed = claimedTiles.Find(prevStepTile) == nullptr;
                if (isPrevStepTileNotClaimed)
                {
                    //  ~?~ Does it make any sence to check if tile included in the path is staticly free (it should be always free)?
                    bool isPrevStepTileFree = passMap->IsTileFree_Safe(prevStepTile,
                                                                       MakeBitMask(EOccupationSlot::OccupationSlot_StaticLowObstacle,
                                                                                   EOccupationSlot::OccupationSlot_StaticHighObstacle));
                    if (isPrevStepTileFree)
                    {
                        FIntPoint ramTile = pushTile;

                        //  Register ram event.
                        actionMovementEvent.mType = EMovementEventType::MoveAndRam;
                        actionMovementEvent.mArenaUnit = action->GetExecutorArenaUnit();
                        if (stepIndex > 0)
                        {
                            actionMovementEvent.mPath.Append(movementPath.GetData(), stepIndex);
                        }

                        useExtraMovementEvent = true;
                        extraMovementEvent.mType = EMovementEventType::Ram;
                        extraMovementEvent.mAction = conflictAction;
                        extraMovementEvent.mArenaUnit = conflictAction->GetExecutorArenaUnit();
                        extraMovementEvent.mPath.Push(pushTile);

                        conflictActions.Add(action);
                        conflictActions.Add(conflictAction);

                        //  Step into previous step tile.
                        tileToClaim = prevStepTile;

                        //  Affect creature:
                        {
                            UBattleLogItem_RamCreature* battleLogItem = nullptr;
                            auto pusherCreature = Cast<ACreatureBase>(action->GetExecutorArenaUnit().GetObject());
                            auto targetCreature = Cast<ACreatureBase>(conflictAction->GetExecutorArenaUnit().GetObject());
                            if (pusherCreature != nullptr &&
                                targetCreature != nullptr)
                            {
                                FIntPoint fromTile = targetCreature->GetTiledMovement()->GetCurrentTile();
                                FIntPoint toTile = pushTile;
                                battleLogItem = battleLog->CreateBattleLogItem_RamCreature(pusherCreature,
                                                                                           targetCreature,
                                                                                           toTile,
                                                                                           FDamageParameters());
                            }
                            actionMovementEvent.mBattleLogItem = battleLogItem;

                            bool isVictimKilled = false;
                            AffectRamVictim(action->GetExecutorActor(),
                                            conflictAction->GetExecutorActor(),
                                            battleLogItem,
                                            isVictimKilled);
                        }

                        break;
                    }
                }
            }
        }

        claimedTiles.FindOrAdd(tileToClaim) = action;

        //  Add movement events:
        actionMovementEvent.mAction = action;
        mMovementEvents.Push(actionMovementEvent);
        if (useExtraMovementEvent)
        {
            mMovementEvents.Push(extraMovementEvent);
        }
    }

    //  All movement conflicts are solved, claimedTiles contains final locations of all actions.
    for (AAction_Base* action : mSubmittedActions)
    {
        const FIntPoint* finalTile = claimedTiles.FindKey(action);
        if (finalTile == nullptr)
        {
            UE_LOG(ArenaCommonLog, Error, TEXT("No claimed tile found for %s. Can't set final tile."),
                   *action->GetName());
        }
        else
        {
            outFinalTiles.Add(action, *finalTile);
        }
    }

    for (FMovementEvent& movementEvent : mMovementEvents)
    {
        movementEvent.mHasConflict = conflictActions.Contains(movementEvent.mAction.Get());
    }
}

void AActionMap_MovementConflictsFirst::ResolveMovementConflicts_StopAtClosestObstacle(TMap<AAction_Base*, FIntPoint>& outFinalTiles)
{
    APassMap* passMap = UArenaUtilities::GetPassMap(this);
    UBattleLog* battleLog = UArenaUtilities::GetBattleLog(this);

    TMap<FIntPoint, AAction_Base*> claimedTiles;
    TSet<AAction_Base*> conflictActions;

    //  Actions which are "pushed" should not perform movement.
    //  ~?~ conflictActions may be used if both push and ram cancells movement.
    TSet<AAction_Base*> cancelledMovementActions;

    //  Initially all actions claims their executors' placements.
    for (AAction_Base* action : mSubmittedActions)
    {
        claimedTiles.Add(action->GetExecutorPlacement(), action);
    }

    for (AAction_Base* action : mSubmittedActions)
    {
        const TArray<FIntPoint>& movementPath = action->GetActionSetupData().mMovementTiles;
        if (movementPath.Num() == 0)
        {
            continue;
        }

        if (cancelledMovementActions.Contains(action))
        {
            continue;
        }

        FMovementEvent actionMovementEvent;
        actionMovementEvent.mAction = action;

        FMovementEvent extraMovementEvent;
        bool useExtraMovementEvent = false;

        FIntPoint tileToClaim = action->GetExecutorPlacement();
        //  If action is moving - remove initially claimed tile to allow pushing other actions to that tile.
        claimedTiles.Remove(tileToClaim);

        FIntPoint prevStepTile = action->GetExecutorPlacement();
        for (int32 stepIndex = 0; stepIndex < movementPath.Num(); ++stepIndex)
        {
            FIntPoint stepTile = movementPath[stepIndex];

            //  Action which already claimed step tile.
            AAction_Base** claimantAction = claimedTiles.Find(stepTile);

            if (claimantAction == nullptr)
            {
                bool isStepTileFree = passMap->IsTileFree_Safe(stepTile,
                                                               MakeBitMask(EOccupationSlot::OccupationSlot_StaticLowObstacle,
                                                                           EOccupationSlot::OccupationSlot_StaticHighObstacle));
                if (isStepTileFree == false)
                {
                    actionMovementEvent.mType = EMovementEventType::Move;
                    actionMovementEvent.mArenaUnit = action->GetExecutorArenaUnit();
                    actionMovementEvent.mPath.Append(movementPath.GetData(), stepIndex + 1);
                    useExtraMovementEvent = false;
                    tileToClaim = prevStepTile;
                    break;
                }

                if (stepIndex == movementPath.Num() - 1)
                {
                    actionMovementEvent.mType = EMovementEventType::Move;
                    actionMovementEvent.mArenaUnit = action->GetExecutorArenaUnit();
                    actionMovementEvent.mPath.Append(movementPath.GetData(), stepIndex + 1);
                    useExtraMovementEvent = false;
                    tileToClaim = stepTile;
                    break;
                }
            }
            else
            {
                AAction_Base* conflictAction = *claimantAction;
                int32 pushDirection = passMap->GetNeighbourDirection(stepTile, prevStepTile);
                FIntPoint pushTile = passMap->GetNeighbour(stepTile, pushDirection);

                bool isPushTileNotClaimed = claimedTiles.Find(pushTile) == nullptr;
                if (isPushTileNotClaimed)
                {
                    bool isPushTileFree = passMap->IsTileFree_Safe(pushTile,
                                                                   MakeBitMask(EOccupationSlot::OccupationSlot_StaticLowObstacle,
                                                                               EOccupationSlot::OccupationSlot_StaticHighObstacle));

                    if (isPushTileFree)
                    {
                        actionMovementEvent.mType = EMovementEventType::MoveAndPush;
                        actionMovementEvent.mArenaUnit = action->GetExecutorArenaUnit();
                        actionMovementEvent.mPath.Append(movementPath.GetData(), stepIndex + 1);

                        useExtraMovementEvent = true;
                        extraMovementEvent.mType = EMovementEventType::Push;
                        extraMovementEvent.mAction = conflictAction;
                        extraMovementEvent.mArenaUnit = conflictAction->GetExecutorArenaUnit();
                        extraMovementEvent.mPath.Push(pushTile);

                        //  Push claiming action to the opposite neighbour tile.
                        claimedTiles.FindOrAdd(pushTile) = conflictAction;

                        conflictActions.Add(action);
                        conflictActions.Add(conflictAction);

                        cancelledMovementActions.Add(conflictAction);

                        //  Step tile can now be claimed by the given action.
                        tileToClaim = stepTile;

                        //  Affect push victim:
                        {
                            UBattleLogItem_PushCreature* battleLogItem = nullptr;
                            auto pusherCreature = Cast<ACreatureBase>(action->GetExecutorArenaUnit().GetObject());
                            auto targetCreature = Cast<ACreatureBase>(conflictAction->GetExecutorArenaUnit().GetObject());
                            if (pusherCreature != nullptr &&
                                targetCreature != nullptr)
                            {
                                FIntPoint fromTile = targetCreature->GetTiledMovement()->GetCurrentTile();
                                FIntPoint toTile = pushTile;
                                battleLogItem = battleLog->CreateBattleLogItem_PushCreature(pusherCreature,
                                                                                            targetCreature,
                                                                                            fromTile,
                                                                                            toTile,
                                                                                            FDamageParameters());
                            }
                            extraMovementEvent.mBattleLogItem = battleLogItem;

                            bool isVictimKilled = false;
                            AffectPushVictim(action->GetExecutorActor(),
                                             conflictAction->GetExecutorActor(),
                                             battleLogItem,
                                             isVictimKilled);

                            if (isVictimKilled)
                            {
                                cancelledMovementActions.Add(conflictAction);
                            }
                        }

                        break;
                    }
                }

                //  As push failed and previous tile was determined to be free for stepping in - register ram event.
                actionMovementEvent.mType = EMovementEventType::MoveAndRam;
                actionMovementEvent.mArenaUnit = action->GetExecutorArenaUnit();
                if (stepIndex > 0)
                {
                    actionMovementEvent.mPath.Append(movementPath.GetData(), stepIndex);
                }

                useExtraMovementEvent = true;
                extraMovementEvent.mType = EMovementEventType::Ram;
                extraMovementEvent.mAction = conflictAction;
                extraMovementEvent.mArenaUnit = (conflictAction)->GetExecutorArenaUnit();
                extraMovementEvent.mPath.Push(pushTile);

                conflictActions.Add(action);
                conflictActions.Add(conflictAction);

                //  Step into previous step tile.
                tileToClaim = prevStepTile;

                //  Affect ram victim:
                {
                    UBattleLogItem_RamCreature* battleLogItem = nullptr;
                    auto pusherCreature = Cast<ACreatureBase>(action->GetExecutorArenaUnit().GetObject());
                    auto targetCreature = Cast<ACreatureBase>(conflictAction->GetExecutorArenaUnit().GetObject());
                    if (pusherCreature != nullptr &&
                        targetCreature != nullptr)
                    {
                        FIntPoint fromTile = targetCreature->GetTiledMovement()->GetCurrentTile();
                        FIntPoint toTile = pushTile;
                        battleLogItem = battleLog->CreateBattleLogItem_RamCreature(pusherCreature,
                                                                                   targetCreature,
                                                                                   toTile,
                                                                                   FDamageParameters());
                    }
                    extraMovementEvent.mBattleLogItem = battleLogItem;

                    bool isVictimKilled = false;
                    AffectRamVictim(action->GetExecutorActor(),
                                    conflictAction->GetExecutorActor(),
                                    battleLogItem,
                                    isVictimKilled);

                    if (isVictimKilled)
                    {
                        cancelledMovementActions.Add(conflictAction);
                    }
                }

                break;
            }

            prevStepTile = stepTile;
        }

        claimedTiles.FindOrAdd(tileToClaim) = action;

        actionMovementEvent.mAction = action;
        mMovementEvents.Push(actionMovementEvent);

        if (useExtraMovementEvent)
        {
            extraMovementEvent.mHasConflict = true;
            mMovementEvents.Push(extraMovementEvent);
        }
    }
        
    for (AAction_Base* action : mSubmittedActions)
    {
        const FIntPoint* claimedTile = claimedTiles.FindKey(action);
        if (claimedTile == nullptr)
        {
            UE_LOG(ArenaCommonLog, Error, TEXT("No claimed tile found for %s. Can't set final tile."),
                   *action->GetName());
        }
        else
        {
            outFinalTiles.Add(action, *claimedTile);
        }
    }

    for (FMovementEvent& movementEvent : mMovementEvents)
    {
        movementEvent.mHasConflict = conflictActions.Contains(movementEvent.mAction.Get());
    }
}

void AActionMap_MovementConflictsFirst::ExecuteMoveEvent(const FMovementEvent& moveEvent,
                                                         const UTiledMovement::FDelegateMovementStopped& delegate)
{
    check(moveEvent.mType == EMovementEventType::Move ||
          moveEvent.mType == EMovementEventType::MoveAndPush ||
          moveEvent.mType == EMovementEventType::MoveAndRam);

    UTiledMovement* passMapMovement = IArenaUnit_Interface::Execute_GetTiledMovement(moveEvent.mArenaUnit.GetObject());
    check(passMapMovement != nullptr && "MoveEvent.mArenaUnit should have UTiledMovement.");

    //  ~?~ Better to move this code out of here to rise attention to this situation?
    if (moveEvent.mPath.Num() == 0)
    {
        delegate.ExecuteIfBound(passMapMovement);
        return;
    }

    APassMap* passMap = passMapMovement->GetPassMap();
    int32 executionDirection = moveEvent.mAction->GetActionSetupData().mExecutionDirection;
    float finalViewAngle = passMap->GetDirectionRotation(executionDirection).Yaw;
    passMapMovement->MovePath(moveEvent.mPath, finalViewAngle);

    if (delegate.IsBound())
    {
        FDelegateHandle delegateHandle = passMapMovement->mEventMovementStopped.Add(delegate);
        mOnMovementFinishedDelegates.Add(passMapMovement, delegateHandle);
    }

    auto movingCreature = Cast<ACreatureBase>(moveEvent.mArenaUnit.GetObject());
    if (movingCreature != nullptr)
    {
        UBattleLog* battleLog = UArenaUtilities::GetBattleLog(this);
        auto battleLogItem = battleLog->CreateBattleLogItem_CreatureMovement(movingCreature,
                                                                             passMapMovement->GetCurrentTile(),
                                                                             moveEvent.mPath.Last());
        battleLog->PushBattleLogItem(battleLogItem);
    }

}

void AActionMap_MovementConflictsFirst::ExecuteMoveAndPushEvent(const FMovementEvent& moveAndPushEvent,
                                                                const FMovementEvent& pushEvent)
{
    check(moveAndPushEvent.mType == EMovementEventType::MoveAndPush);
    check(pushEvent.mType == EMovementEventType::Push);

    UTiledMovement::FDelegateMovementStarted delegate;
    delegate.BindUObject(this, &AActionMap_MovementConflictsFirst::OnConflictMovementFinished);
    ExecuteMoveEvent(moveAndPushEvent, delegate);

    //  ~TODO~ Generate additional animation tasks from pushEvent.
}

void AActionMap_MovementConflictsFirst::ExecuteMoveAndRamEvent(const FMovementEvent& moveAndRamEvent,
                                                               const FMovementEvent& ramEvent)
{
    check(moveAndRamEvent.mType == EMovementEventType::MoveAndRam);
    check(ramEvent.mType == EMovementEventType::Ram);

    UTiledMovement::FDelegateMovementStarted delegate;
    delegate.BindUObject(this, &AActionMap_MovementConflictsFirst::OnConflictMovementFinished);
    ExecuteMoveEvent(moveAndRamEvent, delegate);

    //  ~TODO~ Generate additional animation and damage dealing tasks from ramEvent.
}

void AActionMap_MovementConflictsFirst::OnNonConflictMovementFinished(UTiledMovement* passMapMovement)
{
    OnMovementFinished(passMapMovement);
    OnNonConflictMovementEventExecuted();
}

void AActionMap_MovementConflictsFirst::OnConflictMovementFinished(UTiledMovement* passMapMovement)
{
    check(mMovementEvents.IsValidIndex(mExecutingMovementEventIndex));
    OnMovementFinished(passMapMovement);
    
    const FMovementEvent& movementEvent = mMovementEvents[mExecutingMovementEventIndex];

    if (movementEvent.mType == EMovementEventType::Move)
    {
        OnConflictMovementEventExecuted();
    }
    else if (movementEvent.mType == EMovementEventType::MoveAndPush)
    {
        int32 pushEventIndex = mExecutingMovementEventIndex + 1;
        const FMovementEvent& pushEvent = mMovementEvents[pushEventIndex];
        check(pushEvent.mPath.Num() == 1);
        FIntPoint pushTile = pushEvent.mPath[0];

        AffectPushVictimVisual(movementEvent.mAction->GetExecutorActor(),
                               pushEvent.mAction->GetExecutorActor());

        if (pushEvent.mBattleLogItem != nullptr)
        {
            UBattleLog* battleLog = UArenaUtilities::GetBattleLog(this);
            battleLog->PushBattleLogItem(pushEvent.mBattleLogItem);
        }

        AActor* pushActor = Cast<AActor>(pushEvent.mArenaUnit.GetObject());
        StartPush(pushActor,
                  pushTile);
    }
    else if (movementEvent.mType == EMovementEventType::MoveAndRam)
    {
        int32 ramEventIndex = mExecutingMovementEventIndex + 1;
        const FMovementEvent& ramEvent = mMovementEvents[ramEventIndex];

        AffectRamVictimVisual(movementEvent.mAction->GetExecutorActor(),
                              ramEvent.mAction->GetExecutorActor());

        if (ramEvent.mBattleLogItem != nullptr)
        {
            UBattleLog* battleLog = UArenaUtilities::GetBattleLog(this);
            battleLog->PushBattleLogItem(ramEvent.mBattleLogItem);
        }

        OnConflictMovementEventExecuted();
    }
}

void AActionMap_MovementConflictsFirst::OnMovementFinished(UTiledMovement* passMapMovement)
{
    FDelegateHandle* delegate = mOnMovementFinishedDelegates.Find(passMapMovement);
    if (delegate == nullptr)
    {
        UE_LOG(ArenaCommonLog, Error, TEXT("%s UTiledMovement reported movement finished despite it was not expected."),
               *passMapMovement->GetOwner()->GetName());
        return;
    }
    passMapMovement->mEventMovementStopped.Remove(*delegate);
}

void AActionMap_MovementConflictsFirst::StartPush(AActor* actorToPush,
                                                  const FIntPoint& pushTile)
{
    mActorToPush = actorToPush;
    mPushTile = pushTile;
    mPushStartLocation = actorToPush->GetActorLocation();
    mPushEndLocation = mPassMap->GetTilePositionWorld(pushTile);
    mPushElapsedTime = 0.f;
}

void AActionMap_MovementConflictsFirst::OnPushFinished()
{
    UTiledMovement* passMapMovement = IArenaUnit_Interface::Execute_GetTiledMovement(mActorToPush);
    check(passMapMovement != nullptr);
    passMapMovement->SetLocation(mPushTile);
    mActorToPush = nullptr;

    OnConflictMovementEventExecuted();
}

void AActionMap_MovementConflictsFirst::OnNonConflictMovementEventExecuted()
{
    --mCountOfExecutingNonConflictMoveEvents;
    if (mCountOfExecutingNonConflictMoveEvents <= 0)
    {
        StartExecutingConflictMovementEvents();
    }
}

void AActionMap_MovementConflictsFirst::StartExecutingConflictMovementEvents()
{
    mExecutingMovementEventIndex = -1;
    OnConflictMovementEventExecuted();
}

void AActionMap_MovementConflictsFirst::OnConflictMovementEventExecuted()
{
    while (++mExecutingMovementEventIndex < mMovementEvents.Num())
    {
        const FMovementEvent& movementEvent = mMovementEvents[mExecutingMovementEventIndex];
        if (movementEvent.mHasConflict == false)
        {
            continue;
        }
        if (movementEvent.mType == EMovementEventType::Move)
        {
            UTiledMovement::FDelegateMovementStarted delegate;
            delegate.BindUObject(this, &AActionMap_MovementConflictsFirst::OnConflictMovementFinished);
            ExecuteMoveEvent(movementEvent, delegate);
            return;
        }
        else if (movementEvent.mType == EMovementEventType::MoveAndPush)
        {
            int32 pushEventIndex = mExecutingMovementEventIndex + 1;
            check(pushEventIndex < mMovementEvents.Num());
            ExecuteMoveAndPushEvent(movementEvent, mMovementEvents[pushEventIndex]);
            return;
        }
        else if (movementEvent.mType == EMovementEventType::MoveAndRam)
        {
            int32 ramEventIndex = mExecutingMovementEventIndex + 1;
            check(ramEventIndex < mMovementEvents.Num());
            ExecuteMoveAndRamEvent(movementEvent, mMovementEvents[ramEventIndex]);
            return;
        }
    }

    OnAllConflictMovementEventsExecuted();
}

void AActionMap_MovementConflictsFirst::OnAllConflictMovementEventsExecuted()
{
    //  All movements are executed.
    //  Clear ArenaUnit tile occupations from the previous turn.
    //  Update occupations with the current turn final positions.
    //  Execute actions with the final occupations.

    mExecutingActionIndex = -1;
    OnActionExecuted(nullptr);
}

void AActionMap_MovementConflictsFirst::AffectPushVictim(AActor* pushCauserArenaUnit,
                                                         AActor* pushVictimArenaUnit,
                                                         UBattleLogItem_PushCreature* resultBattleLogItem,
                                                         bool& outIsVictimKilled)
{
    UDamageable* damageable = IArenaUnit_Interface::Execute_GetDamageable(pushVictimArenaUnit);
    if (damageable != nullptr)
    {
        FDamageParameters deliveredDamage;
        damageable->Damage(mPushDamage,
                           pushCauserArenaUnit,
                           deliveredDamage);
        outIsVictimKilled = !damageable->IsAlive();

        if (resultBattleLogItem != nullptr)
        {
            resultBattleLogItem->mPushDamage = deliveredDamage;
        }
    }
}

void AActionMap_MovementConflictsFirst::AffectRamVictim(AActor* ramCauserArenaUnit,
                                                        AActor* ramVictimArenaUnit,
                                                        UBattleLogItem_RamCreature* resultBattleLogItem,
                                                        bool& outIsVictimKilled)
{
    UDamageable* damageable = IArenaUnit_Interface::Execute_GetDamageable(ramVictimArenaUnit);
    if (damageable != nullptr)
    {
        FDamageParameters deliveredDamage;
        damageable->Damage(mRamDamage,
                           ramCauserArenaUnit,
                           deliveredDamage);
        outIsVictimKilled = !damageable->IsAlive();

        if (resultBattleLogItem != nullptr)
        {
            resultBattleLogItem->mRamDamage = deliveredDamage;
        }
    }
}

void AActionMap_MovementConflictsFirst::AffectPushVictimVisual(AActor* pushCauserArenaUnit,
                                                               AActor* pushVictimArenaUnit)
{
    UDamageable* damageable = IArenaUnit_Interface::Execute_GetVisualDamageable(pushVictimArenaUnit);
    if (damageable != nullptr)
    {
        FDamageParameters deliveredDamage;
        damageable->Damage(mPushDamage,
                           pushCauserArenaUnit,
                           deliveredDamage);
    }
}

void AActionMap_MovementConflictsFirst::AffectRamVictimVisual(AActor* ramCauserArenaUnit,
                                                              AActor* ramVictimArenaUnit)
{
    UDamageable* damageable = IArenaUnit_Interface::Execute_GetVisualDamageable(ramVictimArenaUnit);
    if (damageable != nullptr)
    {
        FDamageParameters deliveredDamage;
        damageable->Damage(mRamDamage,
                           ramCauserArenaUnit,
                           deliveredDamage);
    }
}

void AActionMap_MovementConflictsFirst::OnActionExecuted(AAction_Base* executedAction)
{
    ++mExecutingActionIndex;

    if (mSubmittedActions.IsValidIndex(mExecutingActionIndex))
    {
        AAction_Base* action = mSubmittedActions[mExecutingActionIndex];

        //  If action's executor is dead - don't execute the action.
        UDamageable* damageable = IArenaUnit_Interface::Execute_GetDamageable(action->GetExecutorActor());
        if (damageable == nullptr ||
            damageable->IsAlive())
        {
            AAction_SeparateMovement_Base* actionSM = Cast<AAction_SeparateMovement_Base>(action);
            if (actionSM != nullptr)
            {
                actionSM->Apply();
            }
            else
            {
                UE_LOG(ArenaCommonLog, Warning, TEXT("Action %s can't be applied. It doesn't inherit AAction_SeparateMovement_Base."),
                       *action->GetName());
            }

            action->mEventActionExecuted.BindUObject(this, &AActionMap_MovementConflictsFirst::OnActionExecuted);
            action->StartExecution();
        }
        else
        {
            OnActionExecuted(action);
        }
    }
    else
    {
        OnAllActionsExecuted();
    }
}

void AActionMap_MovementConflictsFirst::OnAllActionsExecuted()
{
    //  ~!~R~ Old pipeline style:
    UArenaUtilities::GetTurnsManager(this)->ReleaseTurnExecution(this);
}
