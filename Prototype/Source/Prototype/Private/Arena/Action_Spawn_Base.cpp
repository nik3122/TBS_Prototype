

#include "Arena/Action_Spawn_Base.h"
#include "Arena/ArenaUtilities.h"
#include "Arena/PassMap.h"
#include "Arena/AnimationManager.h"
#include "Arena/TiledMovement.h"
#include "BitmaskOperator.h"
#include "Net/UnrealNetwork.h"


AAction_Spawn_Base::AAction_Spawn_Base(const FObjectInitializer& objectInitializer)
    : Super(objectInitializer)
{
    bReplicates = true;
    bAlwaysRelevant = true;
}

void AAction_Spawn_Base::Reset_Implementation()
{
    Super::Reset_Implementation();
    mSpawnedActors.Empty();
    mIsExecutionFinished = false;
    mIsHiddenOnRep = true;
    mIsSpawnAllowed = true;
}

FActionCollisionResolveResult AAction_Spawn_Base::ResolveCollision_Implementation(AAction_Base* pLowPriorityAction, 
                                                                                  const FIntPoint& collisionPlacement, 
                                                                                  int32 highPrioEntryBehavior, 
                                                                                  int32 lowPrioEntryBehavior)
{
    FActionCollisionResolveResult result = {};
    result.mInterrupt = pLowPriorityAction->IsPretendingOnTile(collisionPlacement) && mIsSpawnAllowed;
    return result;
}

void AAction_Spawn_Base::PreExecutionApply_Implementation()
{
    //  Spawn must be held only on the server:
    if (HasAuthority() &&
        mIsSpawnAllowed &&
        mIsCanceled == false)
    {
        for (FActionDistributionTile& actionEntry : mDistribution)
        {
            if (actionEntry.mTile != GetExecutorPlacement())
            {
                FVector position = UArenaUtilities::GetPassMap(this)->GetTilePositionWorld(actionEntry.mTile);
                FVector viewVector = (GetExecutorActor()->GetActorLocation() - position);
                viewVector.Z = 0.0f;
                FRotator rotation = viewVector.ToOrientationRotator();
                AActor* pSpawnedActor = GetWorld()->SpawnActor(mActorToSpawnClass, &position, &rotation);
                mSpawnedActors.Push(pSpawnedActor);
                UE_LOG(ArenaCommonLog, VeryVerbose, TEXT("%s::PreExecutionApply() spawned %s"), *GetName(), *pSpawnedActor->GetName());
            }
        }
    }
}

void AAction_Spawn_Base::StopExecution()
{
    mIsExecutionFinished = true;
    if (mIsCanceled || mIsSpawnAllowed == false || AreSpawnedActorsReady())
    {
        if (mForceUnhideSpawnedActorsOnStopExecution)
        {
            for (AActor* pSpawnedActor : mSpawnedActors)
            {
                checkSlow(IsValid(pSpawnedActor) && "At this point all spawned actors must be valid.");
                pSpawnedActor->SetActorHiddenInGame(false);
            }
        }
        Super::StopExecution();
    }
}

TArray<UBattleLogItem_Base*> AAction_Spawn_Base::GenerateBattleLogItemsForCurrentTurn_Implementation() const
{
    return {};
}

void AAction_Spawn_Base::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(AAction_Spawn_Base, mSpawnedActors);
}

void AAction_Spawn_Base::OnRep_SpawnedActors()
{
    UE_LOG(ArenaCommonLog, VeryVerbose, TEXT("%s::OnRep_SpawnedActors() spawned actors replicated."), *GetName());

    //  Probably it is called on Reset():
    if (mSpawnedActors.Num() == 0) return;

    check(mIsCanceled == false && "OnRep_SpawnedActors() when action is cancelled.");

    bool allReplicatedActorsAreValid = true;
    for (AActor* pSpawnedActor : mSpawnedActors)
    {
        if (IsValid(pSpawnedActor))
        {
            pSpawnedActor->SetActorHiddenInGame(mIsExecutionFinished == false && mIsHiddenOnRep);
        }
        else
        {
            allReplicatedActorsAreValid = false;
        }
    }
    if (allReplicatedActorsAreValid == false)
    {
        UE_LOG(ArenaCommonLog, Warning, TEXT("%s::OnRep_SpawnedActor() some replicated actor are invalid yet. Waiting for the full replication."), *GetName());
        //  May be "fake" replication only for the array size.
        return;
    }

    //  OnActorsSpawnedLocally is actual for call only during the execution time:
    if (mIsExecuting &&
        mIsExecutionFinished == false)
    {
        OnActorsSpawnedLocally();
    }

    if (mIsExecutionFinished)
    {
        StopExecution();
    }
}

void AAction_Spawn_Base::TrySpawnLocally()
{
    check(mIsExecuting && "TrySpawnLocally() should be called only during the execution.");
    check(mIsSpawnAllowed && mIsCanceled == false && "TrySpawnLocally() spawn is not allowed.");
    for (AActor* pSpawnedActor : mSpawnedActors)
    {
        if (IsValid(pSpawnedActor))
        {
            pSpawnedActor->SetActorHiddenInGame(false);
        }
        else
        {
            UE_LOG(ArenaCommonLog, Warning, TEXT("%s got invalid replicated spawned actor."), *GetName());
        }
    }
    mIsHiddenOnRep = false;
}

bool AAction_Spawn_Base::AreSpawnedActorsReady() const
{
    if (mUseCustomActorReadinessCheck)
    {
        for (AActor* pSpawnedActor : mSpawnedActors)
        {
            if (IsValid(pSpawnedActor) == false ||
                CustomActorReadinessCheck(pSpawnedActor) == false)
            {
                return false;
            }
        }
    }
    else
    {
        for (AActor* pSpawnedActor : mSpawnedActors)
        {
            if (IsValid(pSpawnedActor) == false)
            {
                return false;
            }
        }
    }
    return true;
}

bool AAction_Spawn_Base::CustomActorReadinessCheck_Implementation(AActor* pActor) const
{
    UE_LOG(ArenaCommonLog, Warning, TEXT("%s::CustomActorReadinessCheck() must be implemented."), *GetName());
    return true;
}

bool AAction_Spawn_Base::CanBeSetupAt_Implementation(const FIntPoint& potentialSetupTile) const
{
    return UArenaUtilities::GetPassMap(this)->IsTileFree_Safe(potentialSetupTile, mOccupationSlotsToBeFree, false);
}

bool AAction_Spawn_Base::TryDistributeAt_Implementation(const FIntPoint& potentialDistributionTile, int32& outBehaviorAtDistributionTile)
{
    outBehaviorAtDistributionTile = mActionBehavior;
    return UArenaUtilities::GetPassMap(this)->IsTileFree_Safe(potentialDistributionTile, mOccupationSlotsToBeFree, false);
}

bool AAction_Spawn_Base::FinalizeSetupProcess_Implementation()
{
    return mDistribution.Num() > 1;
}

void AAction_Spawn_Base::OnCancel_Implementation(AAction_Base* pCanceledBy)
{
    //  Interruption tiles are cleared in Action_Base::Cancel().
}

bool AAction_Spawn_Base::OnInterrupt_Implementation(const FIntPoint& interruptionTile,
                                                    AAction_Base* pInterruptedBy)
{
    return interruptionTile == GetExecutorPlacement() || mDistribution.Num() <= 1;
}

bool AAction_Spawn_Base::IsPretendingOnTile_Implementation(const FIntPoint& tile) const
{
    return mDistribution.Contains(tile);
}

void AAction_Spawn_Base::OnStartExecution_Implementation()
{
    mpExecutorTiledMovement->AimAtTile(mPlacement);
    bool stopExecution = true;
    if (mIsCanceled == false &&
        (mIsSpawnAllowed || mPlayAnimationAnyway))
    {
        UAnimationManager* pAnimManager = IArenaUnit_Interface::Execute_GetAnimationManager(GetExecutorActor());
        if (IsValid(pAnimManager) &&
            pAnimManager->TryPlaySingleActionAnimation(mSingleActionAnimationSection))
        {
            pAnimManager->mEventSingleActionAnimationFinished.AddUniqueDynamic(this, &AAction_Spawn_Base::StopExecution);
            stopExecution = false;
        }
    }

    if (stopExecution)
    {
        StopExecution();
    }
}

void AAction_Spawn_Base::OnStopExecution_Implementation()
{
    if (mIsCanceled == false)
    {
        UAnimationManager* pAnimManager = IArenaUnit_Interface::Execute_GetAnimationManager(GetExecutorActor());
        if (IsValid(pAnimManager))
        {
            pAnimManager->mEventSingleActionAnimationFinished.RemoveDynamic(this, &AAction_Spawn_Base::StopExecution);
            pAnimManager->StopAnyAnimationAndSwitchToIdle();
        }
    }
}
