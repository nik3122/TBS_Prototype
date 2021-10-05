#pragma once


#include "CoreMinimal.h"
#include "Arena/BattleLog/BattleLogItem_Base.h"
#include "BattleLogItem_CreatureMovement.generated.h"


UCLASS()
class UBattleLogItem_CreatureMovement
    : public UBattleLogItem_Base
{
    GENERATED_BODY()

public:

    UPROPERTY(BlueprintReadOnly, Category = "BattleLog")
    FBattleLogPartyMemberInfo mCreature;

    UPROPERTY(BlueprintReadOnly, Category = "BattleLog")
    FIntPoint mFromTile;

    UPROPERTY(BlueprintReadOnly, Category = "BattleLog")
    FIntPoint mToTile;
};