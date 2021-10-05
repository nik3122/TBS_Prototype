

#include "Arena/BattleLog/BattleLogItem_ActionAffectsCreature.h"


void UBattleLogItem_ActionAffectsCreature::SetDeliveredDamage(const FDamageParameters& deliveredDamage)
{
    mDamageDelivered = deliveredDamage;
}

void UBattleLogItem_ActionAffectsCreature::AddActivatedStatusEffect(TSubclassOf<AStatusEffect> statusEffectsClass, int32 duration, int32 stage)
{
    FBattleLogStatusEffectInfo statusEffectInfo;
    statusEffectInfo.mStatusEffectClass = statusEffectsClass;
    statusEffectInfo.mDuration = duration;
    statusEffectInfo.mStage = stage;
    mActivatedStatusEffects.Add(statusEffectInfo);
}

void UBattleLogItem_ActionAffectsCreature::AddDeactivatedStatusEffect(TSubclassOf<AStatusEffect> statusEffectClass)
{
    FBattleLogStatusEffectInfo statusEffectInfo;
    statusEffectInfo.mStatusEffectClass = statusEffectClass;
    mActivatedStatusEffects.Add(statusEffectInfo);
}