#pragma once


#include "CoreMinimal.h"
#include "Arena/BattleLog/BattleLogItem_Base.h"
#include "Arena/Damage.h"
#include "BattleLogItem_DamageCreature.generated.h"


UCLASS()
class UBattleLogItem_DamageCreature
    : public UBattleLogItem_Base
{
    GENERATED_BODY()

public:

    UPROPERTY(BlueprintReadOnly, Category = "BattleLog")
    FBattleLogPartyMemberInfo mAttacker;

    UPROPERTY(BlueprintReadOnly, Category = "BattleLog")
    FBattleLogPartyMemberInfo mVictim;

    UPROPERTY(BlueprintReadOnly, Category = "BattleLog")
    FDamageParameters mDamage;
};