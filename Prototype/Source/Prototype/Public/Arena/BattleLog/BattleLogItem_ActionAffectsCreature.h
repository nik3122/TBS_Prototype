#pragma once


#include "CoreMinimal.h"
#include "Arena/BattleLog/BattleLogItem_Base.h"
#include "Arena/Damage.h"
#include "BattleLogItem_ActionAffectsCreature.generated.h"


/*
 *  Describes how an action affects target creature.
 */
UCLASS()
class UBattleLogItem_ActionAffectsCreature
    : public UBattleLogItem_Base
{
    GENERATED_BODY()

public:

    UPROPERTY(BlueprintReadWrite, Category = "BattleLog")
    FBattleLogActionInfo mActionInfo;

    UPROPERTY(BlueprintReadWrite, Category = "BattleLog")
    FBattleLogPartyMemberInfo mTargetCreatureInfo;

    //  Positive values stand for healing, negative stand for damage:
    UPROPERTY(BlueprintReadWrite, Category = "BattleLog")
    FDamageParameters mDamageDelivered;

    UPROPERTY(BlueprintReadWrite, Category = "BattleLog")
    TArray<FBattleLogStatusEffectInfo> mActivatedStatusEffects;

    UPROPERTY(BlueprintReadWrite, Category = "BattleLog")
    TArray<FBattleLogStatusEffectInfo> mDeactivatedStatusEffects;

public:

    void SetDeliveredDamage(const FDamageParameters& deliveredDamage) override;

    void AddActivatedStatusEffect(TSubclassOf<AStatusEffect> statusEffectsClass, int32 duration, int32 stage) override;

    void AddDeactivatedStatusEffect(TSubclassOf<AStatusEffect> statusEffectClass) override;
};
