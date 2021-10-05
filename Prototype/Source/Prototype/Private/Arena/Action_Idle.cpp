
#include "Arena/Action_Idle.h"
#include "UnknownCommon.h"
#include "Arena/ArenaUtilities.h"
#include "Arena/ActionDistributionPattern.h"


AAction_Idle::AAction_Idle()
{
    mActionBehavior = UNKNOWN_IDLE_ACTION_BEHAVIOR;
    mInitiativeBase = 0;
    mReloadTimer = 0;
    mCancellationCounterLimit = MaxCancellationCounter();
    mCancellationCounterChange = 0;
    mIsRecordable = false;
}

bool AAction_Idle::IsAvailableFor(const FIntPoint& actionPlacement) const
{
    return true;
}

FActionCollisionResolveResult AAction_Idle::ResolveCollision_Implementation(AAction_Base* pLowPriorityAction,
                                                                            const FIntPoint& collisionPlacement,
                                                                            int32 highPrioEntryTypes,
                                                                            int32 lowPrioEntryTypes)
{
    FActionCollisionResolveResult result;
    result.mInterrupt = false;
    result.mCancellationCounterChange = 0;
    return result;
}

bool AAction_Idle::IsPretendingOnTile_Implementation(const FIntPoint& tile) const
{
    return tile == GetExecutorPlacement();
}

bool AAction_Idle::IsMakingDamageAtTile_Implementation(const FIntPoint& tile) const
{
    return false;
}

void AAction_Idle::PreExecutionApply_Implementation()
{
    //  Idle action does nothing.
}

void AAction_Idle::PostExecutionApply_Implementation()
{
    //  Idle action does nothing.
}

TArray<UBattleLogItem_Base*> AAction_Idle::GenerateBattleLogItemsForCurrentTurn_Implementation() const
{
    return {};
}

void AAction_Idle::BeginPlay()
{
    Super::BeginPlay();

    //  ~!~ No matter what class will be instanced as it is not used.
    mpDistributionPattern = NewObject<UActionDistributionPattern>(this);
}

bool AAction_Idle::CanBeSetupAt_Implementation(const FIntPoint& potentialSetupTile) const
{
    return true;
}

bool AAction_Idle::TryDistributeAt_Implementation(const FIntPoint& potentialDistributionTile, int32& outBehaviorAtDistributionTile)
{
    return true;
}

bool AAction_Idle::FinalizeSetupProcess_Implementation()
{
    return true;
}

void AAction_Idle::OnStartExecution_Implementation()
{
    //  Idle action does nothing.
    StopExecution();
}

void AAction_Idle::OnStopExecution_Implementation()
{
    //  Idle action does nothing.
}

void AAction_Idle::OnCancel_Implementation(AAction_Base* pCanceledBy)
{
    UE_LOG(ArenaActionsLog, Error, TEXT("Idle action can't be canceled!"));
}

bool AAction_Idle::OnInterrupt_Implementation(const FIntPoint& interruptionTile,
                                              AAction_Base* pInterruptedBy)
{
    check(interruptionTile == GetExecutorPlacement());
    UE_LOG(ArenaActionsLog, Error, TEXT("Idle action can't be!"));
    return false;
}

void AAction_Idle::OnFinishCollisionResolve_Implementation()
{
    //
}
