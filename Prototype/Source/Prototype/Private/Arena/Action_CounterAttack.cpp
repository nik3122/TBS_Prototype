

#include "Arena/Action_CounterAttack.h"
#include "Arena/ArenaUnit_Interface.h"
#include "Arena/ArenaUtilities.h"
#include "Arena/AnimationManager.h"
#include "Arena/BattleLog/BattleLog.h"
#include "Arena/BattleLog/BattleLogItem_ActionAffectsCreature.h"
#include "Arena/PassMap.h"
#include "Arena/TiledMovement.h"
#include "Arena/WeaponOperator.h"
#include "BitmaskOperator.h"


AAction_CounterAttack::AAction_CounterAttack(const FObjectInitializer& initializer)
    : Super(initializer)
{
    mActionBehavior = MakeBitMask(EActionBehavior::ActionBehavior_Defence, EActionBehavior::ActionBehavior_Damage);
    mIsLowOrderInterruptionEnabled = false;
    mIsAOE = false;
    mIsCombat = false;
    mIsRange = false;
    mIsRecordable = false;
    mSingleActionAnimationSection = "CounterAttack";
}

void AAction_CounterAttack::Reset_Implementation()
{
    Super::Reset_Implementation();

    mpTargetDamageable = nullptr;
    mpTargetStatusEffectsManager = nullptr;
    mAttackingActions.Empty();
}

bool AAction_CounterAttack::CanBeSetupAt_Implementation(const FIntPoint& potentialSetupTile) const
{
    return GetExecutorTiledMovement()->GetCurrentTile() == potentialSetupTile;
}

bool AAction_CounterAttack::TryDistributeAt_Implementation(const FIntPoint& potentialDistributionTile,
                                                           int32& outBehaviorAtDistributionTile)
{
    outBehaviorAtDistributionTile = mActionBehavior;
    return true;
}

bool AAction_CounterAttack::FinalizeSetupProcess_Implementation()
{
    return true;
}

FActionCollisionResolveResult AAction_CounterAttack::ResolveCollision(FActionCollisionResolveContext& context,
                                                                      AAction_Base* pLowPriorityAction,
                                                                      const FIntPoint& collisionPlacement,
                                                                      int32 highPrioEntryBehavior,
                                                                      int32 lowPrioEntryBehavior)
{
    if (IsFlagAtBitMask(pLowPriorityAction->GetActionBehaviorsAtTile(collisionPlacement), EActionBehavior::ActionBehavior_Damage))
    {
        mAttackingActions.Push(pLowPriorityAction);
    }
    return {};
}

void AAction_CounterAttack::PreExecutionApply_Implementation()
{
    Super::PreExecutionApply_Implementation();

    if (IsValid(mpTargetDamageable))
    {
        ACreatureBase* pTargetCreautre = Cast<ACreatureBase>(mpTargetDamageable->GetOwner());
        UBattleLog* pBattleLog = UArenaUtilities::GetBattleLog(this);
        UBattleLogItem_Base* pBattleLogItem = nullptr;
        if (pTargetCreautre != nullptr)
        {
            pBattleLogItem = pBattleLog->CreateBattleLogItem_ActionAffectsCreature(this, pTargetCreautre);
        }

        mDamageableTask.mPriority = GetExecutionPriority();
        mDamageableTask.mpResultBattleLogItem = pBattleLogItem;
        mpTargetDamageable->ExecuteDamageableTask(mDamageableTask);

        if (IsValid(mpTargetStatusEffectsManager))
        {
            for (FStatusEffectTask& effectsTask : mStatusEffectTasks)
            {
                effectsTask.mpResultBattleLogItem = pBattleLogItem;
                mpTargetStatusEffectsManager->ExecuteStatusEffectTask(effectsTask);
            }
        }

        if (pTargetCreautre != nullptr)
        {
            pBattleLog->PushBattleLogItems({ pBattleLogItem });
        }
    }
}

void AAction_CounterAttack::OnCancel_Implementation(AAction_Base* pCancelerAction)
{
    mpTargetDamageable = nullptr;
}

bool AAction_CounterAttack::OnInterrupt_Implementation(const FIntPoint& interruptionTile,
                                                       AAction_Base* pInterruptedBy)
{
    //  If counter attack is being interrupted either on target tile or on the executor tile,
    //  it must be cancelled anyway.
    return true;
}

void AAction_CounterAttack::OnFinishCollisionResolve_Implementation()
{
    if (IsCanceled() == false)
    {
        for (AAction_Base* pAttackingAction : mAttackingActions)
        {
            if (pAttackingAction->IsCanceled() == false)
            {
                FIntPoint executorTile = GetExecutorTiledMovement()->GetCurrentTile();
                FIntPoint attackerFinalTile = pAttackingAction->GetExecutorFinalTile();
                //  Check if the final position of the attack is nearby the executor:
                if (GetExecutorTiledMovement()->GetPassMap()->IsNeighbour(executorTile, attackerFinalTile))
                {
                    bool isAttackValid = pAttackingAction->GetDistribution().ContainsByPredicate([executorTile](const FActionDistributionTile& entry) {
                        return entry.mTile == executorTile;
                    });
                    if (isAttackValid)
                    {
                        TScriptInterface<IArenaUnit_Interface> arenaUnit = pAttackingAction->GetExecutorArenaUnit();
                        mpTargetDamageable = IArenaUnit_Interface::Execute_GetDamageable(arenaUnit.GetObject());
                        mpTargetStatusEffectsManager = IArenaUnit_Interface::Execute_GetStatusEffectsManager(arenaUnit.GetObject());
                        break;
                    }
                }
            }
        }
    }
}

void AAction_CounterAttack::OnStartExecution_Implementation()
{
    UAnimationManager* pAnimManager = IArenaUnit_Interface::Execute_GetAnimationManager(GetExecutorActor());
    if (IsCanceled() == false &&
        mpTargetDamageable != nullptr &&
        pAnimManager != nullptr &&
        pAnimManager->TryPlaySingleActionAnimation(mSingleActionAnimationSection))
    {
        pAnimManager->mEventSingleActionAnimationFinished.AddUniqueDynamic(this, &AAction_CounterAttack::StopExecution);
        mpExecutorTiledMovement->AimAt(mpTargetDamageable->GetOwner()->GetActorLocation());

        if (mWeaponClass.Get() != nullptr)
        {
            UWeaponOperator* pWeaponOperator = IArenaUnit_Interface::Execute_GetWeaponOperator(GetExecutorActor());
            if (pWeaponOperator != nullptr)
            {
                AWeapon_Base* pWeapon = pWeaponOperator->SpawnAndEquipWeapon(mWeaponClass);
                check(pWeapon != nullptr);
            }
        }
    }
    else
    {
        StopExecution();
    }
}

void AAction_CounterAttack::OnStopExecution_Implementation()
{
    UAnimationManager* pAnimManager = IArenaUnit_Interface::Execute_GetAnimationManager(GetExecutorActor());
    if (pAnimManager != nullptr)
    {
        pAnimManager->mEventSingleActionAnimationFinished.RemoveDynamic(this, &AAction_CounterAttack::StopExecution);
        pAnimManager->StopAnyAnimationAndSwitchToIdle();
    }
}
