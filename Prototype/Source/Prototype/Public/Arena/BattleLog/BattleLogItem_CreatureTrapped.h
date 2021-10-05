#pragma once


#include "CoreMinimal.h"
#include "Arena/BattleLog/BattleLogItem_Base.h"
#include "Arena/Damage.h"
#include "BattleLogItem_CreatureTrapped.generated.h"


/*
 *
 */
UCLASS()
class UBattleLogItem_CreatureTrapped
    : public UBattleLogItem_Base
{
    GENERATED_BODY()

public:

    UPROPERTY(BlueprintReadWrite, Category = "BattleLog")
    FBattleLogTrapInfo mTrapInfo;

    UPROPERTY(BlueprintReadWrite, Category = "BattleLog")
    FBattleLogPartyMemberInfo mTrappedCreatureInfo;

    //  Positive values stand for healing, negative stand for damage:
    UPROPERTY(BlueprintReadWrite, Category = "BattleLog")
    FDamageParameters mDamageDelivered;

    UPROPERTY(BlueprintReadWrite, Category = "BattleLog")
    TArray<FBattleLogStatusEffectInfo> mActivatedStatusEffects;

    UPROPERTY(BlueprintReadWrite, Category = "BattleLog")
    TArray<FBattleLogStatusEffectInfo> mDeactivatedStatusEffects;
};