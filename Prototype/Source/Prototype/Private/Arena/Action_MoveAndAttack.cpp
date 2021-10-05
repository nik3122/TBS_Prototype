

#include "Arena/Action_MoveAndAttack.h"
#include "Arena/ArenaManager_Interface.h"
#include "Arena/ArenaUtilities.h"
#include "Arena/AnimationManager.h"
#include "Arena/BattleLog/BattleLog.h"
#include "Arena/BattleLog/BattleLogItem_ActionAffectsCreature.h"
#include "Arena/Damageable.h"
#include "Arena/PassMap.h"
#include "Arena/TiledMovement.h"
#include "Arena/WeaponOperator.h"
#include "Arena/Weather.h"
#include "BitmaskOperator.h"


AAction_MoveAndAttack::AAction_MoveAndAttack(const FObjectInitializer& objectInitializer)
    : Super(objectInitializer)
{
    mIsLowOrderInterruptionEnabled = true;
}

void AAction_MoveAndAttack::Reset_Implementation()
{
    Super::Reset_Implementation();
    mTargetArenaUnits.Empty();
}

FActionCollisionResolveResult AAction_MoveAndAttack::ResolveCollision(FActionCollisionResolveContext& context,
                                                                      AAction_Base* pLowPriorityAction,
                                                                      const FIntPoint& collisionPlacement,
                                                                      int32 highPrioEntryBehavior,
                                                                      int32 lowPrioEntryBehavior)
{
    if (IsFlagAtBitMask(lowPrioEntryBehavior, EActionBehavior::ActionBehavior_Executor))
    {
        FActionCollisionResolveResult result = {};
        result.mInterrupt = false;
        result.mCancellationCounterChange = mCancellationCounterChange;
        return result;
    }
    return Super::ResolveCollision(context,
                                   pLowPriorityAction,
                                   collisionPlacement,
                                   highPrioEntryBehavior,
                                   lowPrioEntryBehavior);
}

void AAction_MoveAndAttack::PreExecutionApply_Implementation()
{
    Super::PreExecutionApply_Implementation();

    if (mTargetArenaUnits.Num() > 0)
    {
        //  Update damageable task:
        mDamageableTask.mpInitiator = GetExecutorActor();
        mDamageableTask.mPriority = GetExecutionPriority();

        //  Update damageable effect task:
        for (FStatusEffectTask& effectsTask : mStatusEffectTasks)
        {
            effectsTask.mPriority = GetExecutionPriority();
        }

        UBattleLog* pBattleLog = UArenaUtilities::GetBattleLog(this);
        TArray<UBattleLogItem_Base*> battleLogItems;

        for (AActor* pArenaUnit : mTargetArenaUnits)
        {
            ACreatureBase* pCreature = Cast<ACreatureBase>(pArenaUnit);
            UBattleLogItem_Base* pBattleLogItem = nullptr;
            if (IsValid(pCreature))
            {
                pBattleLogItem = pBattleLog->CreateBattleLogItem_ActionAffectsCreature(this, pCreature);
                battleLogItems.Push(pBattleLogItem);
            }

            UDamageable* pDamageable = IArenaUnit_Interface::Execute_GetDamageable(pArenaUnit);
            if (IsValid(pDamageable))
            {
                mDamageableTask.mPriority = GetExecutionPriority();
                mDamageableTask.mpResultBattleLogItem = pBattleLogItem;
                pDamageable->ExecuteDamageableTask(mDamageableTask);
            }

            UStatusEffectsManager* pStatusEffectsManager = IArenaUnit_Interface::Execute_GetStatusEffectsManager(pArenaUnit);
            if (IsValid(pStatusEffectsManager))
            {
                for (FStatusEffectTask& effectsTask : mStatusEffectTasks)
                {
                    effectsTask.mpResultBattleLogItem = pBattleLogItem;
                    pStatusEffectsManager->ExecuteStatusEffectTask(effectsTask);
                }
            }
        }

        pBattleLog->PushBattleLogItems(battleLogItems);
    }
}

void AAction_MoveAndAttack::PrepareForPlanning_Implementation()
{
    if (mProjectileClass.Get() != nullptr)
    {
        TScriptInterface<IArenaManager_Interface> arenaManagerInterface;
        UArenaUtilities::GetArenaManagerInterface(this, arenaManagerInterface);
        mIsRangeAttackMissing = IArenaManager_Interface::Execute_GetWeather(arenaManagerInterface.GetObject())->GenerateRangeAttackMiss();
    }
}

bool AAction_MoveAndAttack::CanBeSetupAt_Implementation(const FIntPoint& potentialSetupTile) const
{
    if (mpExecutorTiledMovement->GetPassMap()->IsTileFree(potentialSetupTile, MakeBitMask(EOccupationSlot::OccupationSlot_ArenaUnit)))
    {
        return false;
    }
    APassMap* pPassMap = mpExecutorTiledMovement->GetPassMap();
    int32 movementDistance = mpExecutorTiledMovement->GetTileSpeed();
    TArray<FIntPoint> path = pPassMap->FindPath(GetExecutorPlacement(),
                                                potentialSetupTile,
                                                MakeBitMask(EOccupationSlot::OccupationSlot_Static, EOccupationSlot::OccupationSlot_ArenaUnit),
                                                MakeBitMask(EOccupationSlot::OccupationSlot_MovementInterrupter),
                                                movementDistance,
                                                movementDistance,
                                                true);
    return path.Num() > 0;
    //  ~!~ Action_MoveTo doesn't check if tile is reachable or not. Must do it on own or do it in MoveTo (expensive)
    //return mpExecutorTiledMovement->GetPassMap()->IsTileFree(potentialSetupTile, MakeBitMask(EOccupationSlot::OccupationSlot_ArenaUnit)) == false &&
    //    Super::CanBeSetupAt_Implementation(potentialSetupTile);
}

bool AAction_MoveAndAttack::TryDistributeAt_Implementation(const FIntPoint& potentialDistributionTile, int32& outBehaviorAtDistributionTile)
{
    APassMap* pPassMap = mpExecutorTiledMovement->GetPassMap();
    AActor* pOccupant = pPassMap->GetOccupant(potentialDistributionTile, EOccupationSlot::OccupationSlot_ArenaUnit);
    if (IsValid(pOccupant) == false)
    {
        return AAction_MoveTo::TryDistributeAt_Implementation(potentialDistributionTile, outBehaviorAtDistributionTile);
    }
    else if (pOccupant->Implements<UArenaUnit_Interface>())
    {
        mTargetArenaUnits.Push(pOccupant);
        outBehaviorAtDistributionTile = mActionBehavior ^ MakeBitMask(EActionBehavior::ActionBehavior_Move);
        return true;
    }
    return false;
}

bool AAction_MoveAndAttack::FinalizeSetupProcess_Implementation()
{
    //  Action may be executed even if no path exists (target is at neighbor tile):
    bool isMovementSetup = AAction_MoveTo::FinalizeSetupProcess_Implementation();
    UpdateTargetsBasedOnPath();
    return isMovementSetup || mTargetArenaUnits.Num() > 0;
}

bool AAction_MoveAndAttack::OnInterrupt(FActionCollisionResolveContext& context,
                                        const FIntPoint& interruptionTile,
                                        AAction_Base* pInterruptedBy)
{
    bool cancelMovement = mPath.Num() == 0;
    if (mPath.Num() > 0)
    {
        cancelMovement = AAction_MoveTo::OnInterrupt(context, interruptionTile, pInterruptedBy);
        //  mPath may be updated - update targets:
        UpdateTargetsBasedOnPath();
    }
    //  ~HACK~BUG~ When AAction_MoveTo decides to be cancelled we may prevent it if any target arena units are reachable. 
    return cancelMovement && mTargetArenaUnits.Num() == 0;
}

void AAction_MoveAndAttack::OnFinishCollisionResolve_Implementation()
{
    AAction_MoveTo::OnFinishCollisionResolve_Implementation();
    //  ~!~?~ If some of target arena units still executes movement (not cencelled) - damage shouldn't be delivered to it.
    //  ~TODO~ 

    //  mPath may be modified during the collision resolve, thus we need to recheck if all targets are reachable:
    //UpdateTargetsBasedOnPath();
    //if (mPath.Num() == 0 && mTargetArenaUnits.Num() == 0)
    //{
    //    Cancel(this);
    //}
}

void AAction_MoveAndAttack::OnStartExecution_Implementation()
{
    mIsAttackExecutionPhase = false;

    if (mIsCanceled)
    {
        //  ~TODO~ Play cancelation animation.
        StopExecution();
    }
    //  ~!~ If no path - don't execute movement:
    else if (mPath.Num() == 0)
    {
        OnMovementFinished_Implementation();
    }
    else
    {
        Super::OnStartExecution_Implementation();
    }
}

void AAction_MoveAndAttack::OnStopExecution_Implementation()
{
    if (mTargetArenaUnits.Num() > 0)
    {
        mpExecutorTiledMovement->AimAt(mTargetArenaUnits.Last()->GetActorLocation());
    }

    if (mIsAttackExecutionPhase)
    {
        UAnimationManager* pAnimManager = IArenaUnit_Interface::Execute_GetAnimationManager(GetExecutorActor());
        if (pAnimManager != nullptr)
        {
            pAnimManager->mEventSingleActionAnimationFinished.RemoveDynamic(this, &AAction_MoveAndAttack::StopExecution);
            pAnimManager->StopAnyAnimationAndSwitchToIdle();
        }
    }
    else
    {
        AAction_MoveTo::OnStopExecution_Implementation();
    }
}

void AAction_MoveAndAttack::OnMovementFinished_Implementation()
{
    UAnimationManager* pAnimManager = IArenaUnit_Interface::Execute_GetAnimationManager(GetExecutorActor());
    if (mTargetArenaUnits.Num() > 0 &&
        pAnimManager != nullptr &&
        pAnimManager->TryPlaySingleActionAnimation(mSingleActionAnimationSection))
    {
        mIsAttackExecutionPhase = true;
        pAnimManager->mEventSingleActionAnimationFinished.AddUniqueDynamic(this, &AAction_MoveAndAttack::StopExecution);
        mpExecutorTiledMovement->AimAt(mTargetArenaUnits.Last()->GetActorLocation());

        if (mWeaponClass.Get() != nullptr)
        {
            UWeaponOperator* pWeaponOperator = IArenaUnit_Interface::Execute_GetWeaponOperator(GetExecutorActor());
            if (pWeaponOperator != nullptr)
            {
                AWeapon_Base* pWeapon = pWeaponOperator->SpawnAndEquipWeapon(mWeaponClass);
                check(pWeapon != nullptr);
                if (mProjectileClass.Get() != nullptr)
                {
                    pWeapon->LoadProjectile(mProjectileClass);
                    /*for (AActor* pTarget : mTargetArenaUnits)
                    {
                        FWeaponAttackTarget attackTarget;
                        attackTarget.mIsMissing = mIsRangeAttackMissing;
                        attackTarget.mpTargetActor = pTarget;
                        pWeapon->LaunchProjectileAtTarget(attackTarget);
                    }*/
                }
                //  ~TODO~ Set visible damage to be dealed by weapon/projectiles
            }
        }
    }
    else
    {
        StopExecution();
    }
}

TArray<UBattleLogItem_Base*> AAction_MoveAndAttack::GenerateBattleLogItemsForCurrentTurn_Implementation() const
{
    TArray<UBattleLogItem_Base*> battleLogItems;
    UBattleLog* pBattleLog = UArenaUtilities::GetBattleLog(this);
    for (AActor* pTargetActor : mTargetArenaUnits)
    {
        if (ACreatureBase* pTargetCreature = Cast<ACreatureBase>(pTargetActor))
        {
            battleLogItems.Push(pBattleLog->CreateBattleLogItem_ActionAffectsCreature(this,
                pTargetCreature));
        }
    }
    return battleLogItems;
}

void AAction_MoveAndAttack::GenerateAvailableManualSetupTiles(const FIntPoint& baseManualSetupTile, TArray<FActionDistributionTile>& outAvailableManualSetupTiles)
{
    AAction_MoveTo::GenerateAvailableManualSetupTiles(baseManualSetupTile, outAvailableManualSetupTiles);

    const TArray<FActionDistributionTile>& distribution = GetDistribution();
    UTiledMovement* pPassMapMovement = GetExecutorTiledMovement();
    if (distribution.Num() - 1 < pPassMapMovement->GetTileSpeed())
    {
        APassMap* pPassMap = pPassMapMovement->GetPassMap();
        for (const FIntPoint& tile : pPassMap->Neighbors(baseManualSetupTile))
        {
            if (pPassMap->IsTileValid(tile))
            {
                if (pPassMap->IsTileFree(tile, MakeBitMask(EOccupationSlot::OccupationSlot_Static, EOccupationSlot::OccupationSlot_ArenaUnit)))
                {
                    outAvailableManualSetupTiles.Push({ tile, mActionBehavior });
                }
                else
                {
                    AActor* pOccupant = pPassMap->GetOccupant(tile, EOccupationSlot::OccupationSlot_ArenaUnit);
                    if (pOccupant != nullptr && pOccupant->Implements<UArenaUnit_Interface>())
                    {
                        outAvailableManualSetupTiles.Push({ tile, mActionBehavior ^ MakeBitMask(EActionBehavior::ActionBehavior_Move) });
                    }
                }
            }
        }
    }
}

void AAction_MoveAndAttack::UpdateTargetsBasedOnPath()
{
    APassMap* pPassMap = mpExecutorTiledMovement->GetPassMap();
    mTargetArenaUnits.RemoveAllSwap([this, pPassMap](AActor* pTarget) {
        FIntPoint targetTile = IArenaUnit_Interface::Execute_GetTiledMovement(pTarget)->GetCurrentTile();
        if (pPassMap->IsNeighbour(GetExecutorPlacement(), targetTile))
        {
            return false;
        }
        for (const FIntPoint& pathTile : mPath)
        {
            if (pPassMap->IsNeighbour(pathTile, targetTile))
            {
                return false;
            }
        }
        return true;
    });
}
