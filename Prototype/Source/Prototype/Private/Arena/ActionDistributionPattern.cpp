
#include "Arena/ActionDistributionPattern.h"
#include "Arena/Action_Base.h"
#include "Arena/TiledMovement.h"
#include "Arena/PassMap.h"


namespace
{
    FTile EvenToOddShift(const FTile& evenShift)
    {
        return FTile(evenShift.X + (FMath::Abs(evenShift.Y) % 2), evenShift.Y);
    }

    void EvenToOddShifts(const TArray<FTile>& evenShifts, TArray<FTile>& oddShifts)
    {
        oddShifts.Reserve(evenShifts.Num());
        for (const FTile& evenShift : evenShifts)
        {
            oddShifts.Push(EvenToOddShift(evenShift));
        }
    }
}

void UActionDistributionPattern::GetSetupTilesPattern(const FIntPoint& executorPlacement,
                                                      TArray<FIntPoint>& outAvailableSetupTiles,
                                                      const AAction_Base* pAction) const
{
    APassMap* pPassMap = pAction->GetExecutorTiledMovement()->GetPassMap();
    const TArray<FIntPoint>& tileShifts = (executorPlacement.Y % 2) == 0 ? mAvailableTileShiftsEven : mAvailableTileShiftsOdd;
    outAvailableSetupTiles.Reserve(outAvailableSetupTiles.Num() + tileShifts.Num());
    for (const FIntPoint& shift : tileShifts)
    {
        FIntPoint tile = executorPlacement + shift;
        if (pPassMap->IsTileValid(tile))
        {
            outAvailableSetupTiles.Push(tile);
        }
    }
}

void UActionDistributionPattern::GetDistributionTilesPattern(const FIntPoint& executorPlacement,
                                                             const FIntPoint& actionPlacement,
                                                             TArray<FIntPoint>& outAvailableSetupTiles,
                                                             const AAction_Base* pAction) const
{
    FIntPoint actionShift = actionPlacement - executorPlacement;
    const TArray<FActionAvailableDistribution>& availableDistributions = (executorPlacement.Y % 2) == 0 ? mAvailableDistributionsEven : mAvailableDistributionsOdd;
    const FActionAvailableDistribution* pAvailableDistribution = availableDistributions.FindByPredicate([actionShift](const FActionAvailableDistribution& availableDistribution) {
        return availableDistribution.mAvailableTileShifts.Contains(actionShift);
    });
    if (pAvailableDistribution == nullptr)
    {
        return;
    }
    APassMap* pPassMap = pAction->GetExecutorTiledMovement()->GetPassMap();
    TArray<FIntPoint> distributionTilesShifts;
    distributionTilesShifts.Reserve(pAvailableDistribution->mDistributionTileShifts.Num());
    for (const FTile& tileShift : pAvailableDistribution->mDistributionTileShifts)
    {
        FIntPoint tile = executorPlacement + tileShift;
        if (pPassMap->IsTileValid(tile))
        {
            distributionTilesShifts.Push(tile);
        }
    }
    outAvailableSetupTiles.Append(distributionTilesShifts);
}

void UActionDistributionPattern::PostLoad()
{
    Super::PostLoad();

    UpdateInternalData();
}

#if WITH_EDITOR

void UActionDistributionPattern::PostEditChangeProperty(FPropertyChangedEvent& propertyChangedEvent)
{
    Super::PostEditChangeProperty(propertyChangedEvent);

    UpdateInternalData();
}

#endif // #ifdef WITH_EDITO

void UActionDistributionPattern::UpdateInternalData()
{
    mAvailableDistributionsOdd.Empty(mAvailableDistributionsEven.Num());

    for (const FActionAvailableDistribution& avaDistEven : mAvailableDistributionsEven)
    {
        FActionAvailableDistribution& avaDistOdd = mAvailableDistributionsOdd[mAvailableDistributionsOdd.Emplace()];
        EvenToOddShifts(avaDistEven.mAvailableTileShifts, avaDistOdd.mAvailableTileShifts);
        EvenToOddShifts(avaDistEven.mDistributionTileShifts, avaDistOdd.mDistributionTileShifts);
    }

    mAvailableTileShiftsEven.Empty();
    for (const FActionAvailableDistribution& avaDistEven : mAvailableDistributionsEven)
    {
        for (const FTile& evenShift : avaDistEven.mAvailableTileShifts)
        {
            mAvailableTileShiftsEven.Push({ evenShift.X, evenShift.Y });
        }
    }

    mAvailableTileShiftsOdd.Empty();
    for (const FActionAvailableDistribution& avaDistOdd : mAvailableDistributionsOdd)
    {
        for (const FTile& oddShift : avaDistOdd.mAvailableTileShifts)
        {
            mAvailableTileShiftsOdd.Push({ oddShift.X, oddShift.Y });
        }
    }
}

UActionDistributionPattern::UActionDistributionPattern()
{
    //GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Cyan, TEXT("UActionDistributionPattern()"));
}
