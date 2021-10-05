#pragma once


#include "ActionDistributionPattern.h"
#include "ActionDistributionPattern_ArenaUnits.generated.h"


UCLASS()
class UActionDistributionPattern_ArenaUnits
    : public UActionDistributionPattern_Base
{
    GENERATED_BODY()

public:

    void GetSetupTilesPattern(const FIntPoint& executorPlacement,
                              TArray<FIntPoint>& outAvailableSetupTiles,
                              const AAction_Base* pAction) const final;

    void GetDistributionTilesPattern(const FIntPoint& executorPlacement,
                                     const FIntPoint& actionPlacement,
                                     TArray<FIntPoint>& outDistributionTiles,
                                     const AAction_Base* pAction) const final;

protected:

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool mIgnoreAllies = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool mIgnoreEnemies = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool mIncludeCreaturesOnly = true;
};