#pragma once


#include "Tile.h"
#include "ActionDistributionPattern_Base.h"
#include "ActionDistributionPattern.generated.h"


class AAction_Base;


//  Combination of an available action placement shift (relative to executor plcament)
//  and a set of a distribution tiles shifts.
USTRUCT(BlueprintType)
struct FActionAvailableDistribution
{
    GENERATED_BODY()

public:

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FTile> mAvailableTileShifts;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FTile> mDistributionTileShifts;
};


//  ~?~ Better to replace mTileShiftsEvenY and mTileShiftsOddY with a single array of ColumnDependentTile (with members TileEvenY, TileOddY).
//  ~TODO~ Add validation of uniqueness of the indices in the buckets. Non-unique indices doesn't make sence.
UCLASS()
class UActionDistributionPattern
    : public UActionDistributionPattern_Base
{
    GENERATED_BODY()

public:

    UActionDistributionPattern();

    void GetSetupTilesPattern(const FIntPoint& executorPlacement,
                              TArray<FIntPoint>& outAvailableSetupTiles,
                              const AAction_Base* pAction) const override;

    //  ~BUG~SLOW~ As the Pattern doesn't store any state we either have to check if action placement is valid or just rely on valid input.
    //  In some cases validation of action placement may use GetSetupTilesPattern which can be expensive.
    void GetDistributionTilesPattern(const FIntPoint& executorPlacement,
                                     const FIntPoint& actionPlacement,
                                     TArray<FIntPoint>& outDistributionTiles,
                                     const AAction_Base* pAction) const override;

protected:

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FActionAvailableDistribution> mAvailableDistributionsEven;

    UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
    TArray<FActionAvailableDistribution> mAvailableDistributionsOdd;

    TArray<FIntPoint> mAvailableTileShiftsEven;

    TArray<FIntPoint> mAvailableTileShiftsOdd;

protected:

    void PostLoad() override;

#if WITH_EDITOR

    void PostEditChangeProperty(FPropertyChangedEvent& propertyChangedEvent) override;

#endif // #ifdef WITH_EDITOR

    void UpdateInternalData();
};