

#include "Arena/Action_Dash.h"
#include "Arena/ArenaUtilities.h"
#include "Arena/TiledMovement.h"
#include "Arena/PassMap.h"
#include "BitmaskOperator.h"
#include "Arena/CreatureBase.h"
#include "Arena/TrapSystem.h"
#include "Arena/FireSystem.h"


AAction_Dash::AAction_Dash(const FObjectInitializer& objectInitializer)
    : Super(objectInitializer)
{
    mIsLowOrderInterruptionEnabled = false;
    mIsRecordable = false;
    mActionBehavior = MakeBitMask(EActionBehavior::ActionBehavior_Move);
}

void AAction_Dash::Reset_Implementation()
{
    Super::Reset_Implementation();

    mIsDashBlocked = false;
}

void AAction_Dash::StartCollisionResolve(FActionCollisionResolveContext& context)
{

}

FActionCollisionResolveResult AAction_Dash::ResolveCollision_Implementation(AAction_Base* pLowPriorityAction,
                                                                            const FIntPoint& collisionPlacement,
                                                                            int32 highPrioEntryBehavior,
                                                                            int32 lowPrioEntryBehavior)
{
    bool stopClaimingTile = (lowPrioEntryBehavior & MakeBitMask(EActionBehavior::ActionBehavior_Move)) ||
        pLowPriorityAction->IsClaimingTile(collisionPlacement);
    if (stopClaimingTile)
    {
        pLowPriorityAction->StopClaimingTile(collisionPlacement);
    }
    return {};


    //FActionCollisionResolveResult resolveResult = {};
    //bool isPretendingOnTile = IsPretendingOnTile(collisionPlacement);
    //bool interruptMoveTo = (lowPrioEntryTypes & MakeBitMask(EActionBehavior::ActionBehavior_Move)) && isPretendingOnTile;
    //bool interruptOther = isPretendingOnTile && pLowPriorityAction->IsPretendingOnTile(collisionPlacement);
    //resolveResult.mInterrupt = interruptMoveTo || interruptOther;
    //return resolveResult;
}

bool AAction_Dash::IsClaimingTile_Implementation(const FIntPoint& tile) const
{
    return GetPlacement() == tile;
}

void AAction_Dash::StopClaimingTile_Implementation(const FIntPoint& tile)
{
    if (GetPlacement() == tile)
    {
        mIsDashBlocked = true;
    }
}

void AAction_Dash::BlockMovement_Implementation(const FIntPoint& tile,
                                                bool inclusive)
{
    if (GetPlacement() == tile &&
        inclusive)
    {
        mIsDashBlocked = true;
    }
}

void AAction_Dash::PreExecutionApply_Implementation()
{
    ACreatureBase* pExecutorCreature = Cast<ACreatureBase>(GetExecutorActor());
    if (IsValid(pExecutorCreature))
    {
        UTrapSystem* pTrapSystem = UArenaUtilities::GetTrapSystem(this);
        AFireSystem* pFireSystem = UArenaUtilities::GetFireSystem(this);
        pTrapSystem->ProcessMovement({ mPlacement }, pExecutorCreature);
        pFireSystem->ProcessMovement({ mPlacement }, pExecutorCreature);
    }
}

void AAction_Dash::PostExecutionApply_Implementation()
{
    if (mIsCanceled == false)
    {
        //  Teleport to the target position if was not moved (skipped execution).
        mpExecutorTiledMovement->TrySetPlacement(mPlacement);
        mpExecutorTiledMovement->AimAtTile(mPlacement);
    }
}

FIntPoint AAction_Dash::GetExecutorFinalTile() const
{
    return mIsCanceled ? GetExecutorTiledMovement()->GetCurrentTile() : mPlacement;
}

bool AAction_Dash::CanBeSetupAt_Implementation(const FIntPoint& potentialSetupTile) const
{
    //  Rely on ActionDistributionPattern_Teleportation make all work for us.
    return true;
}

bool AAction_Dash::TryDistributeAt_Implementation(const FIntPoint& potentialDistributionTile, int32& outBehaviorAtDistributionTile)
{
    outBehaviorAtDistributionTile = mActionBehavior;
    return potentialDistributionTile == GetPlacement();
}

bool AAction_Dash::FinalizeSetupProcess_Implementation()
{
    return true;
}

void AAction_Dash::OnStartExecution_Implementation()
{
    if (mIsCanceled)
    {
        //  ~TODO~ Play animation of cancelation.
        StopExecution();
    }
    else
    {
        bool setPlacementSucceeded = mpExecutorTiledMovement->TrySetPlacement(GetPlacement());
        check(setPlacementSucceeded);
        //  ~TEMP~TODO~ Don't stop execution at once. 
        StopExecution();
    }
}

void AAction_Dash::OnStopExecution_Implementation()
{
    //  ~TODO~ Stop movement if active.
}

void AAction_Dash::OnCancel_Implementation(AAction_Base* pCanceledBy)
{

}

bool AAction_Dash::OnInterrupt_Implementation(const FIntPoint& interruptionTile,
                                              AAction_Base* pInterruptedBy)
{
    return true;
}

void AAction_Dash::OnFinishCollisionResolve_Implementation()
{

}

bool AAction_Dash::IsPretendingOnTile_Implementation(const FIntPoint& tile) const
{
    return tile == GetPlacement();
}