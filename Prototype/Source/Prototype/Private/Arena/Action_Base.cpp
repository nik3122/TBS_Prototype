// Fill out your copyright notice in the Description page of Project Settings.

#include "Arena/Action_Base.h"
#include "Arena/ArenaUtilities.h"
#include "Arena/PassMap.h"
#include "Arena/ActionMap.h"
#include "Arena/TurnsManager.h"
#include "Arena/ArenaUnit_Interface.h"
#include "Arena/TiledMovement.h"
#include "Arena/ActionDistributionPattern.h"
#include "Arena/BattleLog/BattleLog.h"
#include "BitmaskOperator.h"
#include "Net/UnrealNetwork.h"


AAction_Base::AAction_Base(const FObjectInitializer& objectInitializer)
    : Super(objectInitializer)
{
    PrimaryActorTick.bCanEverTick = false;

    SetCanBeDamaged(false);
    bCollideWhenPlacing = false;
    bEnableAutoLODGeneration = false;
    SetHidden(true);

    bReplicates = true;
    bAlwaysRelevant = true;
}

void AAction_Base::SetActionID(uint32 ActionID)
{
    mActionID = ActionID;
}

uint32 AAction_Base::GetActionID() const
{
    return mActionID;
}

void AAction_Base::PrepareForPlanning_Implementation()
{
    //UE_LOG(ArenaFightActionsLog, Verbose, TEXT("PrepareForPlanning() is not implemented for %s"), *GetClass()->GetName());
}

int32 AAction_Base::GetBehavior() const
{
    return mActionBehavior;
}

bool AAction_Base::DoesSupportMovement() const
{
    return mMovementRangeBase > 0;
}

int32 AAction_Base::GetMovementRange() const
{
    return mMovementRangeBase + mMovementRangeAdditional;
}

int32 AAction_Base::GetMovementRangeBase() const
{
    return mMovementRangeBase;
}

void AAction_Base::SetMovementRangeAdditional(int32 movementRange)
{
    mMovementRangeAdditional = movementRange;
    mEventActionStatsChanged.Broadcast(this);
}

int32 AAction_Base::GetAdditionalMovementRange() const
{
    return mMovementRangeAdditional;
}

int32 AAction_Base::GetInitiative() const
{
    return mInitiativeBase + mInitiativeAdditional;
}

void AAction_Base::SetAdditionalInitiative(int32 initiative)
{
    mInitiativeAdditional = initiative;

    mActionCardInfo.mInitiativeBase = GetInitiative();
    mEventActionStatsChanged.Broadcast(this);
}

int32 AAction_Base::GetAdditionalInitiative() const
{
    return mInitiativeAdditional;
}

int32 AAction_Base::GetRangeMin() const
{
    return mRangeMin;
}

int32 AAction_Base::GetRangeMax() const
{
    return mRangeMax;
}

UTexture2D* AAction_Base::GetIcon() const
{
    return mpIcon;
}

void AAction_Base::SetPlacement(const FIntPoint& placement)
{
    mPlacement = placement;
}

FIntPoint AAction_Base::GetPlacement() const
{
    return mPlacement;
}

void AAction_Base::SetExecutorActor(AActor* pExecutor)
{
    check(IsValid(pExecutor));
    mpExecutorActor = pExecutor;
    mpExecutorUnit = mpExecutorActor;
    check(bool(mpExecutorUnit) && "Executor actor should implement ArenaUnit_Interface.");
    mpExecutorTiledMovement = IArenaUnit_Interface::Execute_GetTiledMovement(mpExecutorActor);
    check(IsValid(mpExecutorTiledMovement) && "Action executor should contain TiledMovement.");
}

AActor* AAction_Base::GetExecutorActor() const
{
    return mpExecutorActor;
}

const TScriptInterface<IArenaUnit_Interface>& AAction_Base::GetExecutorArenaUnit() const
{
    return mpExecutorUnit;
}

UTiledMovement* AAction_Base::GetExecutorTiledMovement() const
{
    return mpExecutorTiledMovement;
}

bool AAction_Base::Setup()
{
    Reset();

    if (mLeftTurnsToReload <= 0 &&
        IsAvailableFor(mPlacement))
    {
        mIsCanceled = false;
        mCancellationCounter = 0;

        TArray<FIntPoint> potentialDistributionTiles;
        GetDistributionTilesPattern(mPlacement, potentialDistributionTiles);
        mDistribution.Reserve(potentialDistributionTiles.Num() + 1);
        mDistribution.SetNum(0, false);
        mDistribution.Push({
            GetExecutorPlacement(),
            MakeBitMask(EActionBehavior::ActionBehavior_Executor),
        });
        for (const FIntPoint& tile : potentialDistributionTiles)
        {
            int32 behaviorAtDistributionTile = 0;
            if (TryDistributeAt(tile, behaviorAtDistributionTile))
            {
                mDistribution.Push(FActionDistributionTile{ tile, behaviorAtDistributionTile });
            }
        }
        mIsSetup = FinalizeSetupProcess();
        if (mIsSetup)
        {
            mSetupTiles.Push(mPlacement);
        }
        UpdateSetupVisualization();
        return mIsSetup;
    }
    UpdateSetupVisualization();
    return false;
}

bool AAction_Base::IsManualSetupEnabled() const
{
    return mIsManualSetupEnabled;
}

void AAction_Base::StartManualSetup()
{
    check(mIsManualSetupEnabled && mIsManualSetupInProgress == false);
    Reset();
    mDistribution.Push({
        GetExecutorPlacement(),
        MakeBitMask(EActionBehavior::ActionBehavior_Executor),
    });
    UpdateAvailableManualSetupTiles();
    mIsManualSetupInProgress = true;
}

bool AAction_Base::IsManualSetupInProgress() const
{
    return mIsManualSetupInProgress;
}

void AAction_Base::CancelManualSetup()
{
    check(mIsManualSetupEnabled && mIsManualSetupInProgress);
    mIsManualSetupInProgress = false;
    Reset();
}

bool AAction_Base::FinishManualSetup()
{
    check(mIsManualSetupEnabled && mIsManualSetupInProgress);
    mIsManualSetupInProgress = false;
    if (mLeftTurnsToReload <= 0 &&
        mDistribution.Num() > 1)
    {
        mIsCanceled = false;
        mCancellationCounter = 0;
        mIsSetup = FinalizeSetupProcess();
    }
    if (mIsSetup == false)
    {
        Reset();
    }
    return mIsSetup;
}

const TArray<FActionDistributionTile>& AAction_Base::GetAvailableManualSetupTiles() const
{
    return mAvailableManualSetupTiles;
}

bool AAction_Base::TrySetNextManualSetupTile(const FIntPoint& tile, bool& canFinishManualSetup, bool& canCancelManualSetup)
{
    check(mIsManualSetupEnabled && mIsManualSetupInProgress);
    bool isTileValid = false;
    if (mAvailableManualSetupTiles.Contains(tile))
    {
        int32 behaviorAtDistributionTile;
        if (TryDistributeAt(tile, behaviorAtDistributionTile))
        {
            mDistribution.Push(FActionDistributionTile{ tile, behaviorAtDistributionTile });
            mSetupTiles.Push(tile);
            isTileValid = true;
        }
    }
    else
    {
        canCancelManualSetup = GetExecutorPlacement() == tile;
        if (canCancelManualSetup == false && mSetupTiles.Num() > 1)
        {
            //  If the user picked tile is already picked previously - cut off all setup and distribution after this tile.
            for (int32 setupLastIndex = mSetupTiles.Num() - 2; mSetupTiles.IsValidIndex(setupLastIndex); --setupLastIndex)
            {
                if (mSetupTiles[setupLastIndex] == tile)
                {
                    //  Cut off setup tiles:
                    TArray<FIntPoint> newSetupTiles(mSetupTiles);
                    newSetupTiles.SetNum(setupLastIndex + 1);
                    //  Reset and setup again the action for the new setup tiles:
                    Reset();
                    mDistribution.Push({
                        GetExecutorPlacement(),
                        MakeBitMask(EActionBehavior::ActionBehavior_Executor),
                    });
                    mSetupTiles = newSetupTiles;
                    for (const FIntPoint& setupTile : mSetupTiles)
                    {
                        int32 behaviorAtDistributionTile;
                        bool isSuccessfull = TryDistributeAt(setupTile, behaviorAtDistributionTile);
                        ensure(isSuccessfull && "It's just a repeat of what has succeeded before at the same order. Should work anyway.");
                        mDistribution.Push(FActionDistributionTile{ setupTile, behaviorAtDistributionTile });
                    }
                    isTileValid = true;
                    break;
                }
            }
        }
    }
    if (isTileValid)
    {
        UpdateAvailableManualSetupTiles();
        canFinishManualSetup = mAvailableManualSetupTiles.Num() == 0;
    }
    return isTileValid;
}

const TArray<FIntPoint>& AAction_Base::GetSetupTiles() const
{
    return mSetupTiles;
}

void AAction_Base::SetMovementPath(const TArray<FIntPoint>& pathTiles)
{
    mSetupData.mMovementTiles = pathTiles;
}

void AAction_Base::SetExecutionDirection(int32 tileDirection)
{
    mSetupData.mExecutionDirection = tileDirection;
}

void AAction_Base::SetActionSetupData(const FActionSetupData& setupData)
{
    mSetupData = setupData;
}

const FActionSetupData& AAction_Base::GetActionSetupData() const
{
    return mSetupData;
}

void AAction_Base::SetFinalTile(const FIntPoint& tile)
{
    mFinalTile = tile;
}

FIntPoint AAction_Base::GetFinalTile() const
{
    return mFinalTile;
}

void AAction_Base::Reset_Implementation()
{
    if (mIsExecuting)
    {
        mIsExecuting = false;
        UE_LOG(ArenaCommonLog, Error, TEXT("Trying to reset %s while its' being executed by %s."),
               *GetClass()->GetName(),
               *GetExecutorActor()->GetName());
    }

    //  ~!~ Don't do reset of the placement as it will be called in the Setup after user set a desired action placement.
    //  May be changed if Setup() accepts action placement so it can be set factually after Reset is called.
    //mPlacement = { -1, -1 };
    mIsSetup = false;
    mIsCanceled = false;
    mCancellationCounter = 0;
    mDistribution.Empty();
    mSetupTiles.Empty();
    mIsCollisionResolveStarted = false;
    mFinalTile = GetExecutorTiledMovement()->GetPassMap()->InvalidTile();

    //  ~!~ Reset() is called at the begginning Setup().
    //  It would be nice to reset SetupData here, but this would prevent the following Setup().
    //  Reset() should be reworked to achieve both types of reset:
    //  1) Reset already setup state of the action.
    //  2) Reset setup data - data which is used for setup.

    UpdateSetupVisualization();
}

bool AAction_Base::IsSetup() const
{
    return mIsSetup;
}

bool AAction_Base::IsAvailableFor(const FIntPoint& actionPlacement) const
{
    return GetAvailableSetupTilesCache().Contains(actionPlacement);
}

int32 AAction_Base::GetReloadTimer() const
{
    return mReloadTimer;
}

int32 AAction_Base::GetLeftTurnsToReload() const
{
    return mLeftTurnsToReload;
}

bool AAction_Base::IsReloaded() const
{
    return GetLeftTurnsToReload() <= 0;
}

void AAction_Base::SetReloaded()
{
    mLeftTurnsToReload = 0;
}

FIntPoint AAction_Base::GetExecutorPlacement() const
{
    return IArenaUnit_Interface::Execute_GetTiledMovement(mpExecutorActor)->GetCurrentTile();
}

const TArray<FActionDistributionTile>& AAction_Base::GetDistribution()
{
    return mDistribution;
}

int32 AAction_Base::GetActionBehaviorsAtTile(const FIntPoint& tile) const
{
    for (auto& entry : mDistribution)
    {
        if (entry.mTile == tile)
        {
            return entry.mEntryBehavior;
        }
    }
    return 0;
}

void AAction_Base::StartCollisionResolve(FActionCollisionResolveContext& context)
{
    mIsCollisionResolveStarted = true;
}

bool AAction_Base::IsCollisionResolveStarted() const
{
    return mIsCollisionResolveStarted;
}

FActionCollisionResolveResult AAction_Base::ResolveCollision(FActionCollisionResolveContext& context,
                                                             AAction_Base* pLowPriorityAction,
                                                             const FIntPoint& collisionPlacement,
                                                             int32 highPrioEntryBehavior,
                                                             int32 lowPrioEntryBehavior)
{
    return ResolveCollision(pLowPriorityAction,
                            collisionPlacement,
                            highPrioEntryBehavior,
                            lowPrioEntryBehavior);
}

FActionCollisionResolveResult AAction_Base::ResolveCollision_Implementation(AAction_Base* pLowPriorityAction,
                                                                            const FIntPoint& collisionPlacement,
                                                                            int32 highPrioEntryBehavior,
                                                                            int32 lowPrioEntryBehavior)
{
    FActionCollisionResolveResult resolveResult = {};
    if (GetExecutorPlacement() == collisionPlacement)
    {
        return resolveResult;
    }

    if (IsPretendingOnTile(collisionPlacement) &&
        pLowPriorityAction->IsPretendingOnTile(collisionPlacement))
    {
        resolveResult.mInterrupt = true;
        return resolveResult;
    }

    if (IsFlagAtBitMask(highPrioEntryBehavior, EActionBehavior::ActionBehavior_Move) &&
        IsFlagAtBitMask(lowPrioEntryBehavior, EActionBehavior::ActionBehavior_Move))
    {
        resolveResult.mInterrupt = true;
        return resolveResult;
    }

    if (IsFlagAtBitMask(highPrioEntryBehavior, EActionBehavior::ActionBehavior_Summon) &&
        IsFlagAtBitMask(lowPrioEntryBehavior, EActionBehavior::ActionBehavior_Summon))
    {
        resolveResult.mInterrupt = true;
        return resolveResult;
    }

    if (IsFlagAtBitMask(highPrioEntryBehavior, EActionBehavior::ActionBehavior_Trap) &&
        IsFlagAtBitMask(lowPrioEntryBehavior, EActionBehavior::ActionBehavior_Trap))
    {
        resolveResult.mInterrupt = true;
        return resolveResult;
    }

    bool isCollidingExecutor = pLowPriorityAction->GetExecutorPlacement() == collisionPlacement;
    if (isCollidingExecutor &&
        IsFlagAtBitMask(highPrioEntryBehavior, EActionBehavior::ActionBehavior_Damage))
    {
        resolveResult.mCancellationCounterChange = GetCancellationCounterChange();
        return resolveResult;
    }

    return resolveResult;
}

void AAction_Base::Cancel(FActionCollisionResolveContext& context,
                          AAction_Base* pCancelerAction)
{
    mIsCanceled = true;
    UE_LOG(ArenaCommonLog, VeryVerbose, TEXT("%s @ %s Cancelled."), *GetExecutorActor()->GetName(), *GetName());
    OnCancel(context, pCancelerAction);
}

bool AAction_Base::IncreaseCancellationCounter(FActionCollisionResolveContext& context,
                                               int32 counterChange,
                                               AAction_Base* pCancelerAction)
{
    mCancellationCounter += counterChange;
    if (mCancellationCounter >= mCancellationCounterLimit)
    {
        Cancel(context, pCancelerAction);
        return true;
    }
    return false;
}

bool AAction_Base::Interrupt(FActionCollisionResolveContext& context,
                             const FIntPoint& interruptionTile,
                             AAction_Base* pHPAction)
{
    mDistribution.RemoveAllSwap([&interruptionTile](const FActionDistributionTile& actionEntry) {
        return actionEntry.mTile == interruptionTile;
    });
    UE_LOG(ArenaCommonLog, Verbose, TEXT("%s @ %s interrupted by %s at [%i;%i]."), *GetExecutorActor()->GetName(), *GetName(), *pHPAction->GetName(), interruptionTile.X, interruptionTile.Y);
    bool cancelAction = OnInterrupt(context, interruptionTile, pHPAction);
    if (cancelAction)
    {
        Cancel(context, pHPAction);
    }
    return cancelAction;
}

bool AAction_Base::IsCanceled() const
{
    return mIsCanceled;
}

int32 AAction_Base::GetCancellationCounterChange() const
{
    return mCancellationCounterChange;
}

int32 AAction_Base::GetCancellationCounterLimit() const
{
    return mCancellationCounterLimit;
}

bool AAction_Base::IsLowOrderInterruptionEnabled() const
{
    return mIsLowOrderInterruptionEnabled;
}

bool AAction_Base::IsClaimingTile_Implementation(const FIntPoint& tile) const
{
    UE_LOG(ArenaCommonLog, Warning, TEXT("IsClaimingTile() is not implemented for %s"), *GetClass()->GetName());
    return false;
}

void AAction_Base::StopClaimingTile_Implementation(const FIntPoint& tile)
{
    UE_LOG(ArenaCommonLog, Warning, TEXT("StopClaimingTile() is not implemented for %s"), *GetClass()->GetName());
}

void AAction_Base::BlockMovement_Implementation(const FIntPoint& collisionTile,
                                                bool inclusive)
{
    UE_LOG(ArenaCommonLog, Warning, TEXT("BlockMovement() is not implemented for %s"), *GetClass()->GetName());
}

void AAction_Base::FinishCollisionResolve()
{
    OnFinishCollisionResolve();
}

bool AAction_Base::IsPretendingOnTile_Implementation(const FIntPoint& tile) const
{
    UE_LOG(ArenaCommonLog, Warning, TEXT("IsPretendingOnTile_Implementation() is not implemented for %s"), *GetClass()->GetName());
    return false;
}

bool AAction_Base::IsMakingDamageAtTile_Implementation(const FIntPoint& tile) const
{
    UE_LOG(ArenaCommonLog, Warning, TEXT("IsMakingDamageAtTile() is not implemented for %s"), *GetClass()->GetName());
    return false;
}

int32 AAction_Base::GetExecutionPriority() const
{
    //check(mIsExecuting && "Execution priority is available only during the execution.");
    return mExecutionPrioirty;
}

void AAction_Base::UpdateAvailableSetupTilesCache()
{
    int32 currentTurn = UArenaUtilities::GetTurnsManager(this)->GetCurrentTurn();
    if (mAvailableTilesCacheUpdateTimeStamp != currentTurn)
    {
        mAvailableSetupTilesCache.SetNum(0, false);
        mpDistributionPattern->GetSetupTilesPattern(GetExecutorPlacement(),
                                                    mAvailableSetupTilesCache,
                                                    this);
        mAvailableSetupTilesCache.RemoveAll([this](const FIntPoint& tile) {
            return !CanBeSetupAt(tile);
        });
        mAvailableTilesCacheUpdateTimeStamp = currentTurn;
    }
}

const TArray<FIntPoint>& AAction_Base::GetAvailableSetupTilesCache() const
{
    const_cast<AAction_Base*>(this)->UpdateAvailableSetupTilesCache();
    return mAvailableSetupTilesCache;
}

void AAction_Base::GetPotentialSetupTiles(TArray<FIntPoint>& outSetupTiles) const
{
    GetSetupTilesPattern(outSetupTiles);
    //  Call to update cache:
    const TArray<FIntPoint>& availableTilesCache = GetAvailableSetupTilesCache();
    outSetupTiles.RemoveAllSwap([availableTilesCache](const FIntPoint& tile) {
        return availableTilesCache.Contains(tile);
    });
}

void AAction_Base::GetSetupTilesPattern(TArray<FIntPoint>& outSetupTiles) const
{
    mpDistributionPattern->GetSetupTilesPattern(GetExecutorPlacement(),
                                                outSetupTiles,
                                                this);
}

void AAction_Base::GetDistributionTilesPattern(const FIntPoint& potentialSetupTile,
                                               TArray<FIntPoint>& outDistributionTiles) const
{
    mpDistributionPattern->GetDistributionTilesPattern(GetExecutorPlacement(),
                                                       potentialSetupTile,
                                                       outDistributionTiles,
                                                       this);
}

void AAction_Base::PreExecutionApply_Implementation()
{
    UE_LOG(ArenaCommonLog, Error, TEXT("%s::PreExecutionApply() should be implemented."), *GetClass()->GetName());
}

void AAction_Base::StartExecution()
{
    check(mIsSetup && "Action must be setup before execution.");
    if (mIsExecuting == false)
    {
        mIsExecuting = true;
        UE_LOG(ArenaCommonLog, VeryVerbose, TEXT("%s @ %s started execution."), *GetExecutorActor()->GetName(), *GetName());
        OnStartExecution();
    }
}

void AAction_Base::StopExecution()
{
    //  Safe to call any time so no "double stop" will appear, e.g. if action has stopped itself earlier than user tried to skip execution.
    if (mIsExecuting)
    {
        mIsExecuting = false;
        UE_LOG(ArenaCommonLog, VeryVerbose, TEXT("%s @ %s stopped execution."), *GetExecutorActor()->GetName(), *GetName());
        OnStopExecution();
        mEventActionExecuted.Execute(this);
    }
}

void AAction_Base::PostExecutionApply_Implementation()
{
    UE_LOG(ArenaCommonLog, Display, TEXT("%s::PostExecutionApply() should be implemented."), *GetClass()->GetName());
}

bool AAction_Base::IsCombat() const
{
    return mIsCombat;
}

bool AAction_Base::IsRange() const
{
    return mIsRange;
}

void AAction_Base::OnConstruction(const FTransform& transform)
{
    InitCardInfo_Implementation(mActionCardInfo);
}

void AAction_Base::InitCardInfo_Implementation(FActionCardInfo& cardInfo)
{
    mActionCardInfo.mBehavior = mActionBehavior;
    mActionCardInfo.mInitiativeBase = mInitiativeBase;
    mActionCardInfo.mReloadTimer = mReloadTimer;
}

void AAction_Base::BeginPlay()
{
    Super::BeginPlay();

    UE_LOG(ArenaCommonLog, Log, TEXT("%s::BegunPlay() owner %s | ID %i."),
           *GetName(),
           *GetOwner()->GetName(),
           mActionID);
}

void AAction_Base::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME_CONDITION(AAction_Base, mActionID, COND_InitialOnly);
}

const FActionCardInfo& AAction_Base::GetActionCardInfo() const
{
    return mActionCardInfo;
}

void AAction_Base::EnableSetupVisualization()
{
    if (mIsSetupVisualizationEnabled)
    {
        UE_LOG(ArenaCommonLog, Log, TEXT("%s::EnableSetupVisualization() already enabled."),
               *GetName());
        return;
    }
    mIsSetupVisualizationEnabled = true;
    EnableSetupVisualization_Inner();
    StartSetupVisualization();
}

void AAction_Base::DisableSetupVisualization()
{
    if (mIsSetupVisualizationEnabled == false)
    {
        UE_LOG(ArenaCommonLog, Log, TEXT("%s::EnableSetupVisualization() already disabled."),
               *GetName());
        return;
    }
    mIsSetupVisualizationEnabled = false;
    DisableSetupVisualization_Inner();
    StopSetupVisualization();
}

void AAction_Base::OnStartExecution_Implementation()
{
    UE_LOG(ArenaCommonLog, Error, TEXT("%s::OnStartExecution() should be implemented."), *GetClass()->GetName());
    //StopExecution();
}

void AAction_Base::OnStopExecution_Implementation()
{
    UE_LOG(ArenaCommonLog, Error, TEXT("%s::OnStopExecution() should be implemented."), *GetClass()->GetName());
}

void AAction_Base::OnCancel(FActionCollisionResolveContext& context, 
                            AAction_Base* pCancelerAction)
{
    OnCancel(pCancelerAction);
}

void AAction_Base::OnCancel_Implementation(AAction_Base* pCancelerAction)
{
    UE_LOG(ArenaCommonLog, Warning, TEXT("%s::OnCancel() should be implemented."), *GetClass()->GetName());
}

bool AAction_Base::OnInterrupt(FActionCollisionResolveContext& context, 
                               const FIntPoint& interruptionTile,
                               AAction_Base* pInterrupterAction)
{
    return OnInterrupt(interruptionTile,
                       pInterrupterAction);
}

bool AAction_Base::OnInterrupt_Implementation(const FIntPoint& interruptionTile,
                                              AAction_Base* pInterrupterAction)
{
    UE_LOG(ArenaCommonLog, Warning, TEXT("%s::OnInterrupt() should be implemented."), *GetClass()->GetName());
    return false;
}

void AAction_Base::OnFinishCollisionResolve_Implementation()
{
    UE_LOG(ArenaCommonLog, Warning, TEXT("%s::OnFinishCollisionResolve() should be implemented."), *GetClass()->GetName());
}

void AAction_Base::UpdateReloadTimer()
{
    mLeftTurnsToReload = FMath::Max(0, --mLeftTurnsToReload);
}

void AAction_Base::SetExecutionPriority(int32 executionPriority)
{
    mExecutionPrioirty = executionPriority;

    //  ~HACK~ We suppose that SetExecutionPriority is called only for actions which would be executed.
    //  We do increment due to the "turns lag":
    mLeftTurnsToReload = mReloadTimer + 1;
}

bool AAction_Base::CanBeSetupAt_Implementation(const FIntPoint& potentialSetupTile) const
{
    UE_LOG(ArenaCommonLog, Error, TEXT("%s::CanBeSetupAt() should be implemented. It should check if potentialSetupTile is suitable for action's setup at the current turn."), *GetClass()->GetName());
    return false;
}

bool AAction_Base::TryDistributeAt_Implementation(const FIntPoint& potentialDistributionTile, int32& outBehaviorAtDistributionTile)
{
    UE_LOG(ArenaCommonLog, Error, TEXT("%s::TryDistributeAt() should be implemented. It should check if potentialDistributionTile is suitable for distribution and if yes - record essential for execution info."), *GetClass()->GetName());
    return false;
}

bool AAction_Base::FinalizeSetupProcess_Implementation()
{
    UE_LOG(ArenaCommonLog, Error, TEXT("%s::FinalizeSetupProcess() should be implemented. Action can complete its' setup here, or cancel it."), *GetClass()->GetName());
    return false;
}

int32 AAction_Base::GetDamageRate() const
{
    check(mDamageRate >= NegativeDamageRate() &&
          mDamageRate <= PositiveDamageRate() && 
          "Damage rate is out of range.");
    return mDamageRate;
}

int32 AAction_Base::GetStatusEffectsRate() const
{
    check(mStatusEffectsRate >= NegativeStatusEffectsRate() && 
          mStatusEffectsRate <= PositiveStatusEffectsRate() && 
          "Status effects rate is out of range.");
    return mStatusEffectsRate;
}

FIntPoint AAction_Base::GetExecutorFinalTile() const
{
    return mpExecutorTiledMovement->GetCurrentTile();
}

TArray<UBattleLogItem_Base*> AAction_Base::GenerateBattleLogItemsForCurrentTurn_Implementation() const
{
    UE_LOG(ArenaCommonLog, Error, TEXT("%s::GenerateBattleLogItemsForCurrentTurn() should be implemented."), *GetClass()->GetName());
    return {};
}

bool AAction_Base::IsRecordable() const
{
    return mIsRecordable;
}

void AAction_Base::UpdateAvailableManualSetupTiles()
{
    mAvailableManualSetupTiles.Empty();
    ensure(mDistribution.Num() > 0);
    GenerateAvailableManualSetupTiles(mDistribution.Top().mTile, mAvailableManualSetupTiles);
    //  Exclude all previously picked tiles:
    for (const FActionDistributionTile& distrTile : mDistribution)
    {
        mAvailableManualSetupTiles.RemoveAllSwap([distrTile](const FActionDistributionTile& stupTile) {
            return distrTile.mTile == stupTile.mTile;
        });
    }
}

void AAction_Base::GenerateAvailableManualSetupTiles(const FIntPoint& baseManualSetupTile, TArray<FActionDistributionTile>& outAvailableManualSetupTiles)
{
    UE_LOG(ArenaCommonLog, Error, TEXT("[%s] AAction_Base::GenerateBattleLogItemsForCurrentTurn() should never be called."), *GetClass()->GetName());
}

void AAction_Base::EnableSetupVisualization_Inner()
{
    //  
}

void AAction_Base::DisableSetupVisualization_Inner()
{
    //
}

void AAction_Base::UpdateSetupVisualization()
{
    //  
}

#if WITH_EDITOR

void AAction_Base::PostEditChangeProperty(FPropertyChangedEvent& propertyChangedEvent)
{
    mActionCardInfo.mpPicture = mpIcon;
    mActionCardInfo.mBehavior = mActionBehavior;
    mActionCardInfo.mReloadTimer = mReloadTimer;
    mActionCardInfo.mInitiativeBase = mInitiativeBase;

    Super::PostEditChangeProperty(propertyChangedEvent);
}

#endif // #if WITH_EDITOR

void AAction_Base::StoreState_Inner(ArenaStateContext& context, State_Action_Base& state) const
{
    FString className = GetClass()->GetName();
    auto classNameANSII = StringCast<ANSICHAR>(*className);
    //  ~!~ Somehow size of the converted string is one char fewer than origin string:
    state.mpClassName = context.AllocText(classNameANSII.Get(), classNameANSII.Length() + 1);
    state.mClassNameSize = classNameANSII.Length();
    if (mSetupTiles.Num() > 0)
    {
        state.mSetupTilesCount = mSetupTiles.Num();
        state.mSetupTiles = context.AllocMemory<FIntPoint>(state.mSetupTilesCount);
        FMemory::Memcpy(state.mSetupTiles, mSetupTiles.GetData(), sizeof(FIntPoint) * state.mSetupTilesCount);
    }
    else
    {
        state.mSetupTiles = nullptr;
        state.mSetupTilesCount = 0;
    }
    state.mIsSetup = mIsSetup;
}

bool AAction_Base::RestoreState_Inner(const State_Action_Base& state)
{
    return true;
}