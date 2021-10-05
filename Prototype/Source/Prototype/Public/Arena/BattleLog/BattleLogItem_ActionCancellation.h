#pragma once


#include "CoreMinimal.h"
#include "Arena/BattleLog/BattleLogItem_Base.h"
#include "BattleLogItem_ActionCancellation.generated.h"


/*
 *
 */
UCLASS()
class UBattleLogItem_ActionCancellation
    : public UBattleLogItem_Base
{
    GENERATED_BODY()

public:

    UPROPERTY(BlueprintReadWrite, Category = "BattleLog")
    FBattleLogActionInfo mCancelledActionInfo;

    UPROPERTY(BlueprintReadWrite, Category = "BattleLog")
    TArray<FBattleLogActionInfo> mCancelledByActionsInfos;
};