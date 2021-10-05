

#include "Arena/BattleLog/BattleLogCommon.h"
#include "Arena/Action_Base.h"
#include "Arena/Party.h"
#include "Arena/SocialComponent.h"
#include "Arena/Trap_Base.h"


FBattleLogPartyMemberInfo::FBattleLogPartyMemberInfo(const ACreatureBase* pCreature)
{
    check(pCreature != nullptr);
    mpCreatureImage = pCreature->GetCreatureImage();
    mPartyMemberColor = pCreature->GetPartyMemberColor();
    if (USocialComponent* pSocial = pCreature->GetSocialComponent())
    {
        if (AParty* pParty = pSocial->GetParty())
        {
            mPartyInfo = pParty->GetPartyInfo();
        }
    }
}

void FBattleLogPartyMemberInfo::SetFromCreature(const ACreatureBase* pCreature)
{
    *this = FBattleLogPartyMemberInfo(pCreature);
}


FBattleLogActionInfo::FBattleLogActionInfo(const AAction_Base* pAction)
{
    check(pAction != nullptr);
    ACreatureBase* pExecutor = Cast<ACreatureBase>(pAction->GetExecutorActor());
    if (pExecutor != nullptr)
    {
        mExecutorInfo.SetFromCreature(pExecutor);
    }
    mpActionImage = pAction->GetIcon();
}

void FBattleLogActionInfo::SetFromAction(const AAction_Base* pAction)
{
    *this = FBattleLogActionInfo(pAction);
}


FBattleLogStatusEffectInfo::FBattleLogStatusEffectInfo(const FStatusEffectTask& statusEffectActivationTask)
{
    mStatusEffectClass = statusEffectActivationTask.mEffectClass;
    mStage = statusEffectActivationTask.mStage;
    mDuration = statusEffectActivationTask.mDuration;
}

void FBattleLogStatusEffectInfo::SetFromStatusEffectActivationTask(const FStatusEffectTask& statusEffectActivationTask)
{
    *this = FBattleLogStatusEffectInfo(statusEffectActivationTask);
}


FBattleLogTrapInfo::FBattleLogTrapInfo(const ATrap_Base* pTrap)
{
    mTrapClass = pTrap->GetClass();
}

void FBattleLogTrapInfo::SetFromTrap(const ATrap_Base* pTrap)
{
    *this = FBattleLogTrapInfo(pTrap);
}
