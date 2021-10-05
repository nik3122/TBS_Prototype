

#include "Arena/ActionDistributionPattern_Sectors.h"
#include "Arena/Action_Base.h"
#include "Arena/PassMap.h"
#include "Arena/TiledMovement.h"


void UActionDistributionPattern_Sectors::GetSetupTilesPattern(const FIntPoint& executorPlacement,
                                                              TArray<FIntPoint>& outAvailableSetupTiles,
                                                              const AAction_Base* pAction) const
{
    int32 minRange = pAction->GetRangeMin();
    int32 maxRange = pAction->GetRangeMax();
    APassMap* pPassMap = pAction->GetExecutorTiledMovement()->GetPassMap();
    pPassMap->GetHexaRing(executorPlacement, minRange, maxRange, outAvailableSetupTiles);
}

void UActionDistributionPattern_Sectors::GetDistributionTilesPattern(const FIntPoint& executorPlacement,
                                                                     const FIntPoint& actionPlacement,
                                                                     TArray<FIntPoint>& outDistributionTiles,
                                                                     const AAction_Base* pAction) const
{
    APassMap* pPassMap = pAction->GetExecutorTiledMovement()->GetPassMap();
    int32 direction = pPassMap->GetDirectionBetweenTiles(executorPlacement, actionPlacement);
    pPassMap->GetHexaSectors(executorPlacement, mSectorsRadius, 
                             direction + mFirstSectorRelativeDirection, 
                             direction + mLastSectorRelativeDirection,
                             outDistributionTiles);
}
