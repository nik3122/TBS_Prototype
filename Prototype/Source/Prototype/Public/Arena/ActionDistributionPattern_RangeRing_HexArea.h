#pragma once


#include "ActionDistributionPattern.h"
#include "ActionDistributionPattern_RangeRing_HexArea.generated.h"


UCLASS()
class UActionDistributionPattern_RangeRing_HexArea
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

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0", ClampMax = "10", UIMin = "0", UIMax = "10"))
    int32 mDistributionAreaRadius = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (Bitmask, BitmaskEnum = "EOccupationSlot"))
    int32 mSetupTilesFreeOccupationSlots = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (Bitmask, BitmaskEnum = "EOccupationSlot"))
    int32 mDistributionTilesFreeOccupationSlots = 0;
};