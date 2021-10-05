#pragma once


#include "ActionDistributionPattern_Base.h"
#include "ActionDistributionPattern_MoveToTarget.generated.h"


UCLASS()
class UActionDistributionPattern_MoveToTarget
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

public:

    //int32
};