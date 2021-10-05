#pragma once


#include "CoreMinimal.h"
#include "Arena/BattleLog/BattleLogItem_Base.h"
#include "Arena/Damage.h"
#include "BattleLogItem_CreatureStatusChange.generated.h"


/*
 *
 */
UCLASS()
class UBattleLogItem_CreatureStatusChange
    : public UBattleLogItem_Base
{
    GENERATED_BODY()

public:

    UPROPERTY(BlueprintReadWrite, Category = "BattleLog")
    FBattleLogPartyMemberInfo mCreatureInfo;

    UPROPERTY(BlueprintReadWrite, Category = "BattleLog")
    int32 mOldHP = 0;

    UPROPERTY(BlueprintReadWrite, Category = "BattleLog")
    int32 mNewHP = 0;

    UPROPERTY(BlueprintReadWrite, Category = "BattleLog")
    FDamageParameters mDeliveredDamage;

    UPROPERTY(BlueprintReadWrite, Category = "BattleLog")
    TArray<FBattleLogStatusEffectInfo> mActivatedStatusEffects;

    UPROPERTY(BlueprintReadWrite, Category = "BattleLog")
    TArray<FBattleLogStatusEffectInfo> mDeactivatedStatusEffects;
};