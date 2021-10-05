#pragma once


#include "CoreMinimal.h"
#include "Arena/BattleLog/BattleLogItem_Base.h"
#include "BattleLogItem_NewTurn.generated.h"


/*
 *  Info about new turn start.
 */
UCLASS()
class UBattleLogItem_NewTurn
    : public UBattleLogItem_Base
{
    GENERATED_BODY()

public:

    UPROPERTY(BlueprintReadWrite, Category = "BattleLog")
    int32 mTurn = -1;
};