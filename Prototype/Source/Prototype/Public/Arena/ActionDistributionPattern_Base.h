#pragma once


#include "ActionDistributionPattern_Base.generated.h"


UCLASS(Abstract)
class UActionDistributionPattern_Base
    : public UDataAsset
{
    GENERATED_BODY()

public:

    UFUNCTION(BlueprintPure)
    virtual void GetSetupTilesPattern(const FIntPoint& executorPlacement,
                                      TArray<FIntPoint>& outAvailableSetupTiles,
                                      const AAction_Base* pAction) const;

    //  ~BUG~SLOW~ As the Pattern doesn't store any state we either have to check if action placement is valid or just rely on valid input.
    //  In some cases validation of action placement may use GetSetupTilesPattern which can be expensive.
    UFUNCTION(BlueprintPure)
    virtual void GetDistributionTilesPattern(const FIntPoint& executorPlacement,
                                             const FIntPoint& actionPlacement,
                                             TArray<FIntPoint>& outDistributionTiles,
                                             const AAction_Base* pAction) const;

protected:

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool mActionRangeAffectsDistribution = false;

};