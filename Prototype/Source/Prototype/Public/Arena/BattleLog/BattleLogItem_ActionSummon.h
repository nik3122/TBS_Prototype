#pragma once


#include "CoreMinimal.h"
#include "Arena/BattleLog/BattleLogItem_Base.h"
#include "BattleLogItem_ActionSummon.generated.h"


/*
 *
 */
UCLASS()
class UBattleLogItem_ActionSummon
    : public UBattleLogItem_Base
{
    GENERATED_BODY()

public:

    UPROPERTY(BlueprintReadWrite, Category = "BattleLog")
    FBattleLogActionInfo mActionInfo;

    UPROPERTY(BlueprintReadWrite, Category = "BattleLog")
    FBattleLogPartyMemberInfo mSummonedCreatureInfo;

    UPROPERTY(BlueprintReadWrite, Category = "BattleLog")
    FIntPoint mTile;
};