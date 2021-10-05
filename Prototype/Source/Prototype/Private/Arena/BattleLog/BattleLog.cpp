
#include "Arena/BattleLog/BattleLog.h"
#include "Arena/Action_Base.h"
#include "Arena/BattleLog/BattleLogItem_ActionAffectsCreature.h"
#include "Arena/BattleLog/BattleLogItem_ActionCancellation.h"
#include "Arena/BattleLog/BattleLogItem_ActionSummon.h"
#include "Arena/BattleLog/BattleLogItem_CreatureDeath.h"
#include "Arena/BattleLog/BattleLogItem_CreatureMovement.h"
#include "Arena/BattleLog/BattleLogItem_CreatureStatusChange.h"
#include "Arena/BattleLog/BattleLogItem_CreatureTrapped.h"
#include "Arena/BattleLog/BattleLogItem_DamageCreature.h"
#include "Arena/BattleLog/BattleLogItem_NewTurn.h"
#include "Arena/BattleLog/BattleLogItem_HealCreature.h"
#include "Arena/BattleLog/BattleLogItem_RamCreature.h"
#include "Arena/BattleLog/BattleLogItem_PushCreature.h"
#include "Arena/CreatureBase.h"
#include "Arena/Damage.h"
#include "Arena/SocialComponent.h"
#include "Arena/StatusEffect.h"
#include "Arena/Trap_Base.h"


//  UBattleLog::

FBattleLogPartyMemberInfo UBattleLog::MakeBattleLogPartyMemberInfo(const ACreatureBase* pCreature)
{
    return FBattleLogPartyMemberInfo(pCreature);
}

FBattleLogActionInfo UBattleLog::MakeBattleLogActionInfo(const AAction_Base* pActionBase)
{
    return FBattleLogActionInfo(pActionBase);
}

FBattleLogStatusEffectInfo UBattleLog::MakeBattleLogStatusEffectInfo(const FStatusEffectTask& statusEffectActivationTask)
{
    return FBattleLogStatusEffectInfo(statusEffectActivationTask);
}

FBattleLogTrapInfo UBattleLog::MakeBattleLogTrapInfo(const ATrap_Base* pTrap)
{
    return FBattleLogTrapInfo(pTrap);
}

void UBattleLog::PushBattleLogItem(UBattleLogItem_Base* pBattleLogItem)
{
    check(pBattleLogItem != nullptr);
    mBattleLog.Push(pBattleLogItem);
    mEventBattleLogUpdated.Broadcast(mBattleLog, 1);
}

void UBattleLog::PushBattleLogItems(TArray<UBattleLogItem_Base*> battleLogItems)
{
    check(battleLogItems.Num() > 0);
    mBattleLog.Append(battleLogItems);
    mEventBattleLogUpdated.Broadcast(mBattleLog, battleLogItems.Num());
}

UBattleLogItem_NewTurn* UBattleLog::CreateBattleLogItem_NewTurn(int32 turn)
{
    auto pBattleLogItem = NewObject<UBattleLogItem_NewTurn>(const_cast<UBattleLog*>(this));
    pBattleLogItem->mTurn = turn;
    return pBattleLogItem;
}

UBattleLogItem_ActionAffectsCreature* UBattleLog::CreateBattleLogItem_ActionAffectsCreature(const AAction_Base* pAction,
                                                                                            const ACreatureBase* pTargetCreature)
{
    auto pBattleLogItem = NewObject<UBattleLogItem_ActionAffectsCreature>(const_cast<UBattleLog*>(this));
    pBattleLogItem->mActionInfo.SetFromAction(pAction);
    pBattleLogItem->mTargetCreatureInfo.SetFromCreature(pTargetCreature);
    return pBattleLogItem;
}

UBattleLogItem_ActionCancellation* UBattleLog::CreateBattleLogItem_ActionCancellation(const AAction_Base* pCancelledAction,
                                                                                      const TArray<AAction_Base*>& cancelledByActions)
{
    auto pBattleLogItem = NewObject<UBattleLogItem_ActionCancellation>(const_cast<UBattleLog*>(this));
    pBattleLogItem->mCancelledActionInfo.SetFromAction(pCancelledAction);
    for (const AAction_Base* pCancelledByAction : cancelledByActions)
    {
        pBattleLogItem->mCancelledByActionsInfos.Emplace(pCancelledByAction);
    }
    return pBattleLogItem;
}

UBattleLogItem_ActionSummon* UBattleLog::CreateBattleLogItem_ActionSummon(const AAction_Base* pAction,
                                                                          const ACreatureBase* pSummonedCreature)
{
    auto pBattleLogItem = NewObject<UBattleLogItem_ActionSummon>(const_cast<UBattleLog*>(this));
    pBattleLogItem->mActionInfo.SetFromAction(pAction);
    pBattleLogItem->mSummonedCreatureInfo.SetFromCreature(pSummonedCreature);
    return pBattleLogItem;
}

UBattleLogItem_CreatureStatusChange* UBattleLog::CreateBattleLogItem_CreatureStatusChange(const ACreatureBase* pCreature,
                                                                                          int32 oldHP,
                                                                                          int32 newHP)
{
    auto pBattleLogItem = NewObject<UBattleLogItem_CreatureStatusChange>(const_cast<UBattleLog*>(this));
    pBattleLogItem->mCreatureInfo.SetFromCreature(pCreature);
    pBattleLogItem->mOldHP = oldHP;
    pBattleLogItem->mNewHP = newHP;
    return pBattleLogItem;
}

UBattleLogItem_CreatureTrapped* UBattleLog::CreateBattleLogItem_CreatureTrapped(const ATrap_Base* pTrap,
                                                                                const ACreatureBase* pTrappedCreature)
{
    auto pBattleLogItem = NewObject<UBattleLogItem_CreatureTrapped>(const_cast<UBattleLog*>(this));
    pBattleLogItem->mTrapInfo.SetFromTrap(pTrap);
    pBattleLogItem->mTrappedCreatureInfo.SetFromCreature(pTrappedCreature);
    return pBattleLogItem;
}

UBattleLogItem_DamageCreature* UBattleLog::CreateBattleLogItem_DamageCreature(ACreatureBase* attacker,
                                                                              ACreatureBase* target,
                                                                              const FDamageParameters& damage)
{
    auto battleLogItem = NewObject<UBattleLogItem_DamageCreature>(this);
    battleLogItem->mAttacker.SetFromCreature(attacker);
    battleLogItem->mVictim.SetFromCreature(target);
    battleLogItem->mDamage = damage;
    return battleLogItem;
}

UBattleLogItem_HealCreature* UBattleLog::CreateBattleLogItem_HealCreature(ACreatureBase* healer,
                                                                          ACreatureBase* target,
                                                                          int32 healedHP)
{
    auto battleLogItem = NewObject<UBattleLogItem_HealCreature>(this);
    battleLogItem->mHealer.SetFromCreature(healer);
    battleLogItem->mTarget.SetFromCreature(target);
    battleLogItem->mHealedHP = healedHP;
    return battleLogItem;
}

UBattleLogItem_CreatureMovement* UBattleLog::CreateBattleLogItem_CreatureMovement(ACreatureBase* creature,
                                                                                  const FIntPoint& fromTile,
                                                                                  const FIntPoint& toTile)
{
    auto battleLogItem = NewObject<UBattleLogItem_CreatureMovement>(this);
    battleLogItem->mCreature.SetFromCreature(creature);
    battleLogItem->mFromTile = fromTile;
    battleLogItem->mToTile = toTile;
    return battleLogItem;
}

UBattleLogItem_RamCreature* UBattleLog::CreateBattleLogItem_RamCreature(ACreatureBase* rammer,
                                                                        ACreatureBase* target,
                                                                        const FIntPoint& ramTile,
                                                                        const FDamageParameters& ramDamage)
{
    auto battleLogItem = NewObject<UBattleLogItem_RamCreature>(this);
    battleLogItem->mRammer.SetFromCreature(rammer);
    battleLogItem->mTarget.SetFromCreature(target);
    battleLogItem->mRamTile = ramTile;
    battleLogItem->mRamDamage = ramDamage;
    return battleLogItem;
}

UBattleLogItem_PushCreature* UBattleLog::CreateBattleLogItem_PushCreature(ACreatureBase* pusher,
                                                                          ACreatureBase* target,
                                                                          const FIntPoint& fromTile,
                                                                          const FIntPoint& toTile,
                                                                          const FDamageParameters& pushDamage)
{
    auto battleLogItem = NewObject<UBattleLogItem_PushCreature>(this);
    battleLogItem->mPusher.SetFromCreature(pusher);
    battleLogItem->mTarget.SetFromCreature(target);
    battleLogItem->mFromTile = fromTile;
    battleLogItem->mToTile = toTile;
    battleLogItem->mPushDamage = pushDamage;
    return battleLogItem;
}

UBattleLogItem_CreatureDeath* UBattleLog::CreateBattleLogItem_CreatureDeath(ACreatureBase* creature)
{
    auto battleLogItem = NewObject<UBattleLogItem_CreatureDeath>(this);
    battleLogItem->mCreature.SetFromCreature(creature);
    return battleLogItem;
}
