#pragma once


#include "ActionDistributionPattern_Base.h"
#include "ActionDistributionPattern_Sectors.generated.h"


UCLASS()
class UActionDistributionPattern_Sectors
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

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "-5", ClampMax = "5", UIMin = "-5", UIMax = "5"))
    int32 mFirstSectorRelativeDirection = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "-5", ClampMax = "5", UIMin = "-5", UIMax = "5"))
    int32 mLastSectorRelativeDirection = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "1", ClampMax = "10", UIMin = "1", UIMax = "10"))
    int32 mSectorsRadius = 1;
};