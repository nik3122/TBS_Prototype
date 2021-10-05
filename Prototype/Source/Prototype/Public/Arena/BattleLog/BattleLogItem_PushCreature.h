#pragma once


#include "CoreMinimal.h"
#include "Arena/BattleLog/BattleLogItem_Base.h"
#include "Arena/Damage.h"
#include "BattleLogItem_PushCreature.generated.h"


UCLASS()
class UBattleLogItem_PushCreature
    : public UBattleLogItem_Base
{
    GENERATED_BODY()

public:

    UPROPERTY(BlueprintReadOnly, Category = "BattleLog")
    FBattleLogPartyMemberInfo mPusher;

    UPROPERTY(BlueprintReadOnly, Category = "BattleLog")
    FBattleLogPartyMemberInfo mTarget;

    UPROPERTY(BlueprintReadOnly, Category = "BattleLog")
    FDamageParameters mPushDamage;

    UPROPERTY(BlueprintReadOnly, Category = "BattleLog")
    FIntPoint mFromTile;

    UPROPERTY(BlueprintReadOnly, Category = "BattleLog")
    FIntPoint mToTile;
};