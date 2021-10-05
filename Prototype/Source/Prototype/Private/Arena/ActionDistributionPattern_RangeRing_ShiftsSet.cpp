
#include "Arena/ActionDistributionPattern_RangeRing_ShiftsSet.h"
#include "Arena/Action_Base.h"
#include "Arena/PassMap.h"
#include "Arena/TiledMovement.h"


void UActionDistributionPattern_RangeRing_ShiftsSet::GetSetupTilesPattern(const FIntPoint& executorPlacement,
                                                                          TArray<FIntPoint>& outAvailableSetupTiles,
                                                                          const AAction_Base* pAction) const
{
    int32 minRange = pAction->GetRangeMin();
    int32 maxRange = pAction->GetRangeMax();
    APassMap* pPassMap = pAction->GetExecutorTiledMovement()->GetPassMap();
    TArray<FIntPoint> resultTiles;
    pPassMap->GetHexaRing(executorPlacement, minRange, maxRange, resultTiles);
    if (mSetupTilesFreeOccupationSlots != 0)
    {
        resultTiles.RemoveAll([this, pPassMap](const FIntPoint& tile) {
            return !pPassMap->IsTileFree(tile, mSetupTilesFreeOccupationSlots);
        });
    }
    outAvailableSetupTiles.Append(resultTiles);
}

void UActionDistributionPattern_RangeRing_ShiftsSet::GetDistributionTilesPattern(const FIntPoint& executorPlacement,
                                                                                 const FIntPoint& actionPlacement,
                                                                                 TArray<FIntPoint>& outDistributionTiles,
                                                                                 const AAction_Base* pAction) const
{
    APassMap* pPassMap = pAction->GetExecutorTiledMovement()->GetPassMap();
    const TArray<FTile>& distributionTilesShifts = actionPlacement.Y % 2 ? mDistribuionTilesShiftsOdd : mDistribuionTilesShiftsEven;
    for (const FTile& shift : distributionTilesShifts)
    {
        FIntPoint tile = actionPlacement + shift;
        if (pPassMap->IsTileValid(tile))
        {
            outDistributionTiles.Push(tile);
        }
    }
}

#if WITH_EDITOR

void UActionDistributionPattern_RangeRing_ShiftsSet::PostEditChangeProperty(FPropertyChangedEvent& propertyChangedEvent)
{
    mDistribuionTilesShiftsOdd.Empty(mDistribuionTilesShiftsEven.Num());
    FTile::EvenToOddTiles(mDistribuionTilesShiftsEven, mDistribuionTilesShiftsOdd);
}

#endif // #ifdef WITH_EDITO
