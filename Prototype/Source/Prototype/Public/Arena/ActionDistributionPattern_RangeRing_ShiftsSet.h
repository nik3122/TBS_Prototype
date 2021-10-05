#pragma once


#include "ActionDistributionPattern_Base.h"
#include "Tile.h"
#include "ActionDistributionPattern_RangeRing_ShiftsSet.generated.h"


UCLASS()
class UActionDistributionPattern_RangeRing_ShiftsSet
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
    TArray<FTile> mDistribuionTilesShiftsEven;

    UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
    TArray<FTile> mDistribuionTilesShiftsOdd;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (Bitmask, BitmaskEnum = "EOccupationSlot"))
    int32 mSetupTilesFreeOccupationSlots = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (Bitmask, BitmaskEnum = "EOccupationSlot"))
    int32 mDistributionTilesFreeOccupationSlots = 0;

protected:

#if WITH_EDITOR

    void PostEditChangeProperty(FPropertyChangedEvent& propertyChangedEvent) override;

#endif // #ifdef WITH_EDITOR

};