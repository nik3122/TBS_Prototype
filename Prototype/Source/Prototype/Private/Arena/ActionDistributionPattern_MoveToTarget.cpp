

#include "Arena/ActionDistributionPattern_MoveToTarget.h"
#include "Arena/Action_Base.h"
#include "Arena/TiledMovement.h"
#include "Arena/PassMap.h"
#include "BitmaskOperator.h"


void UActionDistributionPattern_MoveToTarget::GetSetupTilesPattern(const FIntPoint& executorPlacement, TArray<FIntPoint>& outAvailableSetupTiles, const AAction_Base* pAction) const
{
    UTiledMovement* pTiledMovement = pAction->GetExecutorTiledMovement();
    APassMap* pPassMap = pTiledMovement->GetPassMap();
    pPassMap->GetHexaArea(executorPlacement, pTiledMovement->GetTileSpeed(), outAvailableSetupTiles);
    outAvailableSetupTiles.RemoveSingleSwap(executorPlacement);
}

void UActionDistributionPattern_MoveToTarget::GetDistributionTilesPattern(const FIntPoint& executorPlacement, const FIntPoint& actionPlacement, TArray<FIntPoint>& outDistributionTiles, const AAction_Base* pAction) const
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
                                                true);
    outDistributionTiles.Append(path);
}

