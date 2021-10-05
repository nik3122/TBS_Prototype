
#include "Arena/Action_MoveTo.h"
#include "Arena/ArenaUtilities.h"
#include "Arena/ActionCollisionResolveContext.h"
#include "Arena/PassMap.h"
#include "Arena/TiledMovement.h"
#include "Arena/TrapSystem.h"
#include "Arena/FireSystem.h"
#include "Arena/CreatureBase.h"
#include "BitmaskOperator.h"


AAction_MoveTo::AAction_MoveTo(const FObjectInitializer& objectInitializer)
    : Super(objectInitializer)
{
    mIsLowOrderInterruptionEnabled = false;
    mIsRecordable = false;
    mActionBehavior = MakeBitMask(EActionBehavior::ActionBehavior_Move);
    mIsManualSetupEnabled = true;
}

void AAction_MoveTo::Reset_Implementation()
{
    Super::Reset_Implementation();
    mPath.Empty();
    mPathStatus.Empty();
    mDestTileIndex = INDEX_NONE;
}

void AAction_MoveTo::StartCollisionResolve(FActionCollisionResolveContext& context)
{
    Super::StartCollisionResolve(context);

    int32 pathIndex = mPathStatus.Num() - 1;
    while (pathIndex >= 0)
    {
        if (mPathStatus[pathIndex])
        {
            break;
        }
        --pathIndex;
    }
    if (pathIndex != INDEX_NONE)
    {
        const FIntPoint& destTile = mPath[pathIndex];
        mPath.SetNum(pathIndex + 1, false);
        context.SetTilePretender(destTile, this);
    }
    else
    {
        //  No free tiles left - empty path:
        mPath.SetNum(0, false);
    }
    return;
}

FActionCollisionResolveResult AAction_MoveTo::ResolveCollision(FActionCollisionResolveContext& context,
                                                               AAction_Base* pLowPriorityAction,
                                                               const FIntPoint& collisionPlacement,
                                                               int32 highPrioEntryBehavior,
                                                               int32 lowPrioEntryBehavior)
{
    FActionCollisionResolveResult resolveResult = {};

    int32 pathIndex = mPath.Find(collisionPlacement);
    if (pathIndex == mDestTileIndex)
    {
        bool stopClaimingTile = (lowPrioEntryBehavior & MakeBitMask(EActionBehavior::ActionBehavior_Move)) ||
            pLowPriorityAction->IsClaimingTile(collisionPlacement);
        if (stopClaimingTile)
        {
            pLowPriorityAction->StopClaimingTile(collisionPlacement);
        }
    }
    return resolveResult;

    //

    //return {};

    //

    //{
    //    FActionCollisionResolveResult resolveResult = {};
    //
    //    bool isPretendingOnTile = IsPretendingOnTile(collisionPlacement);
    //    bool interruptMovement = (lowPrioEntryBehavior & MakeBitMask(EActionBehavior::ActionBehavior_Move)) && isPretendingOnTile;
    //    bool interruptOther = isPretendingOnTile && pLowPriorityAction->IsPretendingOnTile(collisionPlacement);
    //
    //    //  If collision tile is the same as the current pretending tile - finally maintain it in the context
    //    //  and interrupt all movement actions bellow in the collision stack. All other tiles doesn't matter for us.
    //    int32 collisionTileIndex = mPath.FindLast(collisionPlacement);
    //    if (collisionTileIndex == mDestTileIndex)
    //    {
    //        resolveResult.mInterrupt = interruptMovement || interruptOther;
    //    }
    //    return resolveResult;
    //}
}

bool AAction_MoveTo::IsClaimingTile_Implementation(const FIntPoint& tile) const
{
    int32 pathIndex = mPath.Find(tile);
    return pathIndex != INDEX_NONE && mPathStatus[pathIndex];
}

void AAction_MoveTo::StopClaimingTile_Implementation(const FIntPoint& tile)
{
    int32 pathIndex = mPath.Find(tile);
    if (pathIndex != INDEX_NONE)
    {
        mPathStatus[pathIndex] = false;
    }
}

void AAction_MoveTo::BlockMovement_Implementation(const FIntPoint& tile,
                                                  bool inclusive)
{
    int32 pathIndex = mPath.Find(tile);
    if (pathIndex != INDEX_NONE)
    {
        pathIndex += inclusive ? 0 : 1;
        while (pathIndex < mPathStatus.Num())
        {
            mPathStatus[pathIndex] = false;
            ++pathIndex;
        }
    }
}

void AAction_MoveTo::PreExecutionApply_Implementation()
{
    ACreatureBase* pExecutorCreature = Cast<ACreatureBase>(GetExecutorActor());
    if (IsValid(pExecutorCreature))
    {
        UTrapSystem* pTrapSystem = UArenaUtilities::GetTrapSystem(this);
        AFireSystem* pFireSystem = UArenaUtilities::GetFireSystem(this);
        pTrapSystem->ProcessMovement(mPath, pExecutorCreature);
        pFireSystem->ProcessMovement(mPath, pExecutorCreature);
    }
}

void AAction_MoveTo::PostExecutionApply_Implementation()
{
    if (mIsCanceled == false &&
        mPath.Num() > 0 &&
        mpExecutorTiledMovement->GetCurrentTile() != mPath.Last())
    {
        //  Teleport to the target position if was not moved (skipped execution).
        mpExecutorTiledMovement->TrySetPlacement(mPath.Last());
    }
}

FIntPoint AAction_MoveTo::GetExecutorFinalTile() const
{
    return mPath.Num() > 0 ? mPath.Last() : mpExecutorTiledMovement->GetCurrentTile();
}

bool AAction_MoveTo::CanBeSetupAt_Implementation(const FIntPoint& potentialSetupTile) const
{
    //  Just rely on full ActionDistributionPattern_MoveTo validness.
    return true;
}

bool AAction_MoveTo::TryDistributeAt_Implementation(const FIntPoint& potentialDistributionTile, int32& outBehaviorAtDistributionTile)
{
    //  ~!~HACK~ We rely on ActionDistributionPattern_MoveTo ordering distribution tiles from starting tile to the target tile.
    mPath.Push(potentialDistributionTile);
    outBehaviorAtDistributionTile = mActionBehavior;
    return true;
}

bool AAction_MoveTo::FinalizeSetupProcess_Implementation()
{
    if (mpExecutorTiledMovement->IsStunned() ||
        mPath.Num() == 0)
    {
        return false;
    }

    mDestTileIndex = mPath.Num() - 1;

    //  Update status for whole path:
    mPathStatus.SetNum(mPath.Num());
    for (auto& tileStatus : mPathStatus)
    {
        tileStatus = true;
    }

    return true;
}

void AAction_MoveTo::OnCancel(FActionCollisionResolveContext& context,
                              AAction_Base* pCancelerAction)
{
    Super::OnCancel(pCancelerAction);
}

bool AAction_MoveTo::OnInterrupt(FActionCollisionResolveContext& context,
                                 const FIntPoint& interruptionTile,
                                 AAction_Base* pInterrupterAction)
{
    const int32 interruptionTileIndex = mPath.Find(interruptionTile);
    if (interruptionTileIndex != INDEX_NONE)
    {
        mPathStatus[interruptionTileIndex] = false;
    }
    return false;

    ////  On interruption the destination tile can be changed to:
    ////  0) Not changed if being interrupted in the middle of the path by actions pretending on interruption tile.
    ////  1) Set to interruption tile if being interrupted not by actions pretending on the interrupted actions ~HACK~.
    ////  2) Set to the last valid tile in the path before the interruption tile if interruption was at the destination tile.

    //const int32 interruptionTileIndex = mPath.Find(interruptionTile);
    ////  We assume that if interruption tile is not found - it is already interrupted:
    //if (interruptionTileIndex == INDEX_NONE)
    //{
    //    return false;
    //}
    //int32 lastValidTileIndex = INDEX_NONE;
    //bool pickLastValidTile = false;

    //if (context.IsTilePretended(interruptionTile))
    //{
    //    //  Destination tile is the last in the path, check it for interruption:
    //    if (interruptionTileIndex == (mPath.Num() - 1))
    //    {
    //        lastValidTileIndex = interruptionTileIndex - 1;
    //        pickLastValidTile = true;
    //    }
    //    mPathStatus[interruptionTileIndex] = false;
    //}
    //else
    //{
    //    /*In case we have collision with other MoveTo but not at it's destination tile we have this branch active.
    //    We need to check if the collision is for MoveTo or other action.*/
    //    lastValidTileIndex = interruptionTileIndex;
    //    pickLastValidTile = true;
    //}

    //if (pickLastValidTile)
    //{
    //    while (lastValidTileIndex >= 0 &&
    //           mPathStatus[lastValidTileIndex] == false)
    //    {
    //        --lastValidTileIndex;
    //    }
    //    mPath.SetNum(lastValidTileIndex + 1, false);
    //}

    //return mPath.Num() == 0;
}

void AAction_MoveTo::OnFinishCollisionResolve_Implementation()
{
    //	Nothing to do here. All changes are being done "in live", when solving the collisions.
}

bool AAction_MoveTo::IsPretendingOnTile_Implementation(const FIntPoint& tile) const
{
    return mPath.Num() > 0 && mPath.Last() == tile;
}

void AAction_MoveTo::OnStartExecution_Implementation()
{
    if (mPath.Num() == 0 ||
        mIsCanceled)
    {
        //  ~TODO~ Play animation of cancelation.
        StopExecution();
    }
    else
    {
        mpExecutorTiledMovement->mOnStop.AddUniqueDynamic(this, &AAction_MoveTo::OnMovementFinished);
        bool isMoving = mpExecutorTiledMovement->MovePath(mPath, 0.0f);
        check(isMoving && "TiledMovement can't move the specified path.");
    }
}

void AAction_MoveTo::OnStopExecution_Implementation()
{
    //  ~TODO~ Stop movement.
    mpExecutorTiledMovement->mOnStop.RemoveDynamic(this, &AAction_MoveTo::OnMovementFinished);
}

void AAction_MoveTo::OnMovementFinished_Implementation()
{
    StopExecution();
}

void AAction_MoveTo::GenerateAvailableManualSetupTiles(const FIntPoint& baseManualSetupTile,
                                                       TArray<FActionDistributionTile>& outAvailableManualSetupTiles)
{
    const TArray<FActionDistributionTile>& distribution = GetDistribution();
    UTiledMovement* pPassMapMovement = GetExecutorTiledMovement();
    if (distribution.Num() - 1 < pPassMapMovement->GetTileSpeed())
    {
        APassMap* pPassMap = pPassMapMovement->GetPassMap();
        for (const FIntPoint& tile : pPassMap->Neighbors(baseManualSetupTile))
        {
            int32 occupationMask = MakeBitMask(EOccupationSlot::OccupationSlot_Static,
                                               EOccupationSlot::OccupationSlot_ArenaUnit,
                                               EOccupationSlot::OccupationSlot_StaticLowObstacle,
                                               EOccupationSlot::OccupationSlot_StaticHighObstacle);
            if (pPassMap->IsTileValid(tile) &&
                pPassMap->IsTileFree(tile, occupationMask))
            {
                outAvailableManualSetupTiles.Push({ tile, mActionBehavior });
            }
        }
    }
}

void AAction_MoveTo::UpdatePretendingTile(FActionCollisionResolveContext& context)
{
    context.FreeTilePretender(mPath[mDestTileIndex], this);
    while (mDestTileIndex >= 0)
    {
        FIntPoint pathTile = mPath[mDestTileIndex];
        if (context.IsTilePretended(pathTile) == false)
        {
            context.SetTilePretender(pathTile, this);
        }
        --mDestTileIndex;
    }
}
