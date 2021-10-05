#pragma once


#include "Action_Base.h"
#include "Action_MoveTo.generated.h"


UCLASS(Blueprintable)
class AAction_MoveTo
    : public AAction_Base
{
    GENERATED_BODY()

public:

    AAction_MoveTo(const FObjectInitializer& objectInitializer = FObjectInitializer::Get());

    ~AAction_MoveTo() = default;

    void Reset_Implementation() override;

    void StartCollisionResolve(FActionCollisionResolveContext& context) override;

    FActionCollisionResolveResult ResolveCollision(FActionCollisionResolveContext& context,
                                                   AAction_Base* pLowPriorityAction,
                                                   const FIntPoint& collisionPlacement,
                                                   int32 highPrioEntryTypes,
                                                   int32 lowPrioEntryTypes) override;

    bool IsClaimingTile_Implementation(const FIntPoint& tile) const override;

    void StopClaimingTile_Implementation(const FIntPoint& tile) override;

    void BlockMovement_Implementation(const FIntPoint& tile,
                                      bool inclusive) override;

    void PreExecutionApply_Implementation() override;

    void PostExecutionApply_Implementation() override;

    FIntPoint GetExecutorFinalTile() const override;

protected:

    UPROPERTY(BlueprintReadWrite)
    TArray<FIntPoint> mPath;

    TArray<bool> mPathStatus;

    int32 mDestTileIndex = 0;

protected:

    bool CanBeSetupAt_Implementation(const FIntPoint& potentialSetupTile) const override;

    bool TryDistributeAt_Implementation(const FIntPoint& potentialDistributionTile, int32& outBehaviorAtDistributionTile) override;

    bool FinalizeSetupProcess_Implementation() override;

    void OnStartExecution_Implementation() override;

    void OnStopExecution_Implementation() override;

    void OnCancel(FActionCollisionResolveContext& context,
                  AAction_Base* pCancelerAction) override;

    bool OnInterrupt(FActionCollisionResolveContext& context, 
                     const FIntPoint& interruptionTile,
                     AAction_Base* pInterrupterAction) override;

    void OnFinishCollisionResolve_Implementation() override;

    bool IsPretendingOnTile_Implementation(const FIntPoint& tile) const override;

    UFUNCTION(BlueprintNativeEvent, Category = "Arena")
    void OnMovementFinished();
    virtual void OnMovementFinished_Implementation();

    void GenerateAvailableManualSetupTiles(const FIntPoint& baseManualSetupTile, TArray<FActionDistributionTile>& outAvailableManualSetupTiles) override;

private:

    void UpdatePretendingTile(FActionCollisionResolveContext& context);
};