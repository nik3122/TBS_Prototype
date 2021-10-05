#pragma once


#include "CoreMinimal.h"
#include "Arena/BattleLog/BattleLogItem_Base.h"
#include "BattleLogItem_CreatureDeath.generated.h"


UCLASS()
class UBattleLogItem_CreatureDeath
    : public UBattleLogItem_Base
{
    GENERATED_BODY()

public:

    UPROPERTY(BlueprintReadOnly, Category = "BattleLog", Meta = (DisplayName = "Creature"))
    FBattleLogPartyMemberInfo mCreature;
};