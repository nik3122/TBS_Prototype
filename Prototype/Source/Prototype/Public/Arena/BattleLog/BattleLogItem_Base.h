#pragma once


#include "CoreMinimal.h"
#include "Arena/BattleLog/BattleLogCommon.h"
#include "BattleLogItem_Base.generated.h"


UCLASS(BlueprintType, Abstract)
class UBattleLogItem_Base
    : public UObject
{
    GENERATED_BODY()

public:

    virtual void SetDeliveredDamage(const FDamageParameters& deliveredDamage) {};

    virtual void AddActivatedStatusEffect(TSubclassOf<AStatusEffect> statusEffectsClass, int32 duration, int32 stage) {};

    virtual void AddDeactivatedStatusEffect(TSubclassOf<AStatusEffect> statusEffectClass) {};
};