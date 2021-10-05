#pragma once


#include "ActionDistributionPattern.h"
#include "ActionDistributionPattern_RangeRing_RandomChain.generated.h"


UCLASS()
class UActionDistributionPattern_RangeRing_RandomChain
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

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0", UIMin = "0"))
    int32 mChainLength = 0;
};