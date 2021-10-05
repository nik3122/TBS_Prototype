#pragma once


#include "Action_MoveTo.h"
#include "ArenaUnit_Interface.h"
#include "Damage.h"
#include "StatusEffectsManager.h"
#include "Weapon_Base.h"
#include "Projectile_Base.h"
#include "Action_MoveAndAttack.generated.h"


UCLASS()
class AAction_MoveAndAttack
    : public AAction_MoveTo
{
    GENERATED_BODY()

public:

    AAction_MoveAndAttack(const FObjectInitializer& objectInitializer = FObjectInitializer::Get());

    void Reset_Implementation() override;

    FActionCollisionResolveResult ResolveCollision(FActionCollisionResolveContext& context,
                                                   AAction_Base* pLowPriorityAction,
                                                   const FIntPoint& collisionPlacement,
                                                   int32 highPrioEntryBehavior,
                                                   int32 lowPrioEntryBehavior) override;

    void PreExecutionApply_Implementation() override;

    TArray<UBattleLogItem_Base*> GenerateBattleLogItemsForCurrentTurn_Implementation() const override;

protected:

    // ~?~!~ May be changed to the array of damageable tasks.
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Arena")
    FDamageableTask mDamageableTask;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Arena")
    TArray<FStatusEffectTask> mStatusEffectTasks;

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Arena")
    FName mSingleActionAnimationSection;

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Arena")
    TSubclassOf<AWeapon_Base> mWeaponClass;

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Arena")
    TSubclassOf<AProjectile_Base> mProjectileClass;

    UPROPERTY(BlueprintReadOnly, Category = "Arena")
    TArray<AActor*> mTargetArenaUnits;

    bool mIsAttackExecutionPhase = false;

    bool mIsRangeAttackMissing = false;

protected:

    void PrepareForPlanning_Implementation() override;

    bool CanBeSetupAt_Implementation(const FIntPoint& potentialSetupTile) const override;

    bool TryDistributeAt_Implementation(const FIntPoint& potentialDistributionTile, int32& outBehaviorAtDistributionTile) override;

    bool FinalizeSetupProcess_Implementation() override;

    bool OnInterrupt(FActionCollisionResolveContext& context,
                     const FIntPoint& interruptionTile,
                     AAction_Base* pInterruptedBy) override;

    void OnFinishCollisionResolve_Implementation() override;

    void OnStartExecution_Implementation() override;

    void OnStopExecution_Implementation() override;

    void OnMovementFinished_Implementation() override;

    void GenerateAvailableManualSetupTiles(const FIntPoint& baseManualSetupTile, TArray<FActionDistributionTile>& outAvailableManualSetupTiles) override;

private:

    //  Must be called each time mPath is changed. ~?~ Maybe it worth to introduce AAction_MoveTo::OnPathChanged() to update targets there:
    void UpdateTargetsBasedOnPath();

};