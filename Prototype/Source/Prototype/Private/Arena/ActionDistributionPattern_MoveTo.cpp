
#include "Arena/ActionDistributionPattern_MoveTo.h"
#include "Arena/Action_Base.h"
#include "Arena/TiledMovement.h"
#include "Arena/PassMap.h"
#include "BitmaskOperator.h"


void UActionDistributionPattern_MoveTo::GetSetupTilesPattern(const FIntPoint& executorPlacement,
                                                             TArray<FIntPoint>& outAvailableSetupTiles,
                                                             const AAction_Base* pAction) const
{
    UTiledMovement* pTiledMovement = pAction->GetExecutorTiledMovement();
    APassMap* pPassMap = pTiledMovement->GetPassMap();
    pPassMap->GetReachableTiles(executorPlacement,
                                pTiledMovement->GetTileSpeed(),
                                MakeBitMask(EOccupationSlot::OccupationSlot_Static,
                                            EOccupationSlot::OccupationSlot_ArenaUnit,
                                            EOccupationSlot::OccupationSlot_StaticLowObstacle,
                                            EOccupationSlot::OccupationSlot_StaticHighObstacle),
                                outAvailableSetupTiles);
}

void UActionDistributionPattern_MoveTo::GetDistributionTilesPattern(const FIntPoint& executorPlacement,
                                                                    const FIntPoint& actionPlacement,
                                                                    TArray<FIntPoint>& outDistributionTiles,
                                                                    const AAction_Base* pAction) const
{
    UTiledMovement* pTiledMovement = pAction->GetExecutorTiledMovement();
    APassMap* pPassMap = pTiledMovement->GetPassMap();
    int32 movementDistance = pTiledMovement->GetTileSpeed();
    TArray<FIntPoint> path = pPassMap->FindPath(executorPlacement,
                                                actionPlacement,
                                                MakeBitMask(EOccupationSlot::OccupationSlot_Static,
                                                            EOccupationSlot::OccupationSlot_ArenaUnit,
                                                            EOccupationSlot::OccupationSlot_StaticLowObstacle,
                                                            EOccupationSlot::OccupationSlot_StaticHighObstacle),
                                                MakeBitMask(EOccupationSlot::OccupationSlot_MovementInterrupter),
                                                movementDistance,
                                                movementDistance,
                                                mIgnoreTargetTileOccupation);
    outDistributionTiles.Append(path);
}
