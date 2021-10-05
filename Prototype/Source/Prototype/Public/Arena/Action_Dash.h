#pragma once


#include "Action_Base.h"
#include "Action_Dash.generated.h"


UCLASS()
class AAction_Dash
    : public AAction_Base
{
    GENERATED_BODY()

public:

    AAction_Dash(const FObjectInitializer& objectInitializer = FObjectInitializer::Get());

    void Reset_Implementation() final;

    void StartCollisionResolve(FActionCollisionResolveContext& context) final;

    FActionCollisionResolveResult ResolveCollision_Implementation(AAction_Base* pLowPriorityAction,
                                                                  const FIntPoint& collisionPlacement,
                                                                  int32 highPrioEntryBehavior,
                                                                  int32 lowPrioEntryBehavior) final;

    bool IsClaimingTile_Implementation(const FIntPoint& tile) const override;

    void StopClaimingTile_Implementation(const FIntPoint& tile) override;

    void BlockMovement_Implementation(const FIntPoint& tile,
                                      bool inclusive) override;

    void PreExecutionApply_Implementation() final;

    void PostExecutionApply_Implementation() final;

    FIntPoint GetExecutorFinalTile() const final;

protected:

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Unknown")
    int32 mActionRange = 0;

    bool mIsDashBlocked = false;

protected:

    bool CanBeSetupAt_Implementation(const FIntPoint& potentialSetupTile) const final;

    bool TryDistributeAt_Implementation(const FIntPoint& potentialDistributionTile, int32& outBehaviorAtDistributionTile) final;

    bool FinalizeSetupProcess_Implementation() final;

    void OnStartExecution_Implementation() final;

    void OnStopExecution_Implementation() final;

    void OnCancel_Implementation(AAction_Base* pCanceledBy) final;

    bool OnInterrupt_Implementation(const FIntPoint& interruptionTile,
                                    AAction_Base* pInterruptedBy) final;

    void OnFinishCollisionResolve_Implementation() final;

    bool IsPretendingOnTile_Implementation(const FIntPoint& tile) const final;
};