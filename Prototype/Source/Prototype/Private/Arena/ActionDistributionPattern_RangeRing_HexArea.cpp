
#include "Arena/ActionDistributionPattern_RangeRing_HexArea.h"
#include "Arena/Action_Base.h"
#include "Arena/PassMap.h"
#include "Arena/TiledMovement.h"


void UActionDistributionPattern_RangeRing_HexArea::GetSetupTilesPattern(const FIntPoint& executorPlacement,
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

void UActionDistributionPattern_RangeRing_HexArea::GetDistributionTilesPattern(const FIntPoint& executorPlacement, 
                                                                               const FIntPoint& actionPlacement, 
                                                                               TArray<FIntPoint>& outDistributionTiles, 
                                                                               const AAction_Base* pAction) const
{
    APassMap* pPassMap = pAction->GetExecutorTiledMovement()->GetPassMap();
    TArray<FIntPoint> resultTiles;
    pPassMap->GetHexaArea(actionPlacement, mDistributionAreaRadius, resultTiles);
    if (mDistributionTilesFreeOccupationSlots != 0)
    {
        resultTiles.RemoveAll([this, pPassMap](const FIntPoint& tile) {
            return !pPassMap->IsTileFree(tile, mDistributionTilesFreeOccupationSlots);
        });
    }
    outDistributionTiles.Append(resultTiles);
}

