#pragma once


#include "CoreMinimal.h"
#include "Arena/BattleLog/BattleLogItem_Base.h"
#include "BattleLogItem_HealCreature.generated.h"


UCLASS()
class UBattleLogItem_HealCreature
    : public UBattleLogItem_Base
{
    GENERATED_BODY()

public:

    UPROPERTY(BlueprintReadOnly, Category = "BattleLog")
    FBattleLogPartyMemberInfo mHealer;

    UPROPERTY(BlueprintReadOnly, Category = "BattleLog")
    FBattleLogPartyMemberInfo mTarget;

    UPROPERTY(BlueprintReadOnly, Category = "BattleLog")
    int32 mHealedHP = 0;
};