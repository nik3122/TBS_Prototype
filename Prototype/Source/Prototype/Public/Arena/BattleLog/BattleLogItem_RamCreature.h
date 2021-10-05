#pragma once


#include "CoreMinimal.h"
#include "Arena/BattleLog/BattleLogItem_Base.h"
#include "Arena/Damage.h"
#include "BattleLogItem_RamCreature.generated.h"


UCLASS()
class UBattleLogItem_RamCreature
    : public UBattleLogItem_Base
{
    GENERATED_BODY()

public:

    UPROPERTY(BlueprintReadOnly, Category = "BattleLog")
    FBattleLogPartyMemberInfo mRammer;

    UPROPERTY(BlueprintReadOnly, Category = "BattleLog")
    FBattleLogPartyMemberInfo mTarget;

    UPROPERTY(BlueprintReadOnly, Category = "BattleLog")
    FIntPoint mRamTile;

    UPROPERTY(BlueprintReadOnly, Category = "BattleLog")
    FDamageParameters mRamDamage;
};