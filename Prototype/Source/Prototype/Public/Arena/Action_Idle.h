#pragma once

#include "CoreMinimal.h"
#include "Action_Base.h"
#include "Action_Idle.generated.h"


UCLASS()
class AAction_Idle
    : public AAction_Base
{
    GENERATED_BODY()

public:

    AAction_Idle();

    bool IsAvailableFor(const FIntPoint& actionPlacement) const final;

    FActionCollisionResolveResult ResolveCollision_Implementation(AAction_Base* pLowPriorityAction,
                                                                  const FIntPoint& collisionPlacement,
                                                                  int32 highPrioEntryTypes,
                                                                  int32 lowPrioEntryTypes) final;

    bool IsPretendingOnTile_Implementation(const FIntPoint& tile) const final;

    bool IsMakingDamageAtTile_Implementation(const FIntPoint& tile) const final;

    void PreExecutionApply_Implementation() final;

    void PostExecutionApply_Implementation() final;

    TArray<UBattleLogItem_Base*> GenerateBattleLogItemsForCurrentTurn_Implementation() const final;

protected:

    void BeginPlay() final;

    bool CanBeSetupAt_Implementation(const FIntPoint& potentialSetupTile) const final;

    bool TryDistributeAt_Implementation(const FIntPoint& potentialDistributionTile, int32& outBehaviorAtDistributionTile) final;

    bool FinalizeSetupProcess_Implementation() final;

    void OnStartExecution_Implementation() final;

    void OnStopExecution_Implementation() final;

    void OnCancel_Implementation(AAction_Base* pCanceledBy) final;

    bool OnInterrupt_Implementation(const FIntPoint& interruptionTile,
                                    AAction_Base* pInterruptedBy) final;

    void OnFinishCollisionResolve_Implementation() final;
};