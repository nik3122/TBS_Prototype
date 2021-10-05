#pragma once


#include "Arena/CreatureBase.h"
#include "ArenaUnitShopInfo.generated.h"


USTRUCT(BlueprintType)
struct FArenaUnitShopInfo
    : public FTableRowBase
{
    GENERATED_BODY()

public:

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ArenaLobby")
    FText mUnitNameLoc;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ArenaLobby")
    int32 mCost = 0;

    UPROPERTY(BlueprintReadWrite, Category = "ArenaLobby")
    int32 mCount = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ArenaLobby")
    int32 mMaxCount = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ArenaLobby")
    TSubclassOf<ACreatureBase> mCreatureClass;
};