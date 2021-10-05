#pragma once


#include "Action_Base.h"
#include "Damageable.h"
#include "StatusEffectsManager.h"
#include "Weapon_Base.h"
#include "Action_CounterAttack.generated.h"


class UDamageable;


UCLASS(Blueprintable)
class AAction_CounterAttack
    : public AAction_Base
{
    GENERATED_BODY()

public:

    AAction_CounterAttack(const FObjectInitializer& initializer = FObjectInitializer::Get());

    void Reset_Implementation() override;

    FActionCollisionResolveResult ResolveCollision(FActionCollisionResolveContext& context,
                                                   AAction_Base* pLowPriorityAction,
                                                   const FIntPoint& collisionPlacement,
                                                   int32 highPrioEntryBehavior,
                                                   int32 lowPrioEntryBehavior) override;

    void PreExecutionApply_Implementation() override;

protected:

    // ~?~!~ May be changed to the array of damageable tasks.
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Arena")
    FDamageableTask mDamageableTask;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Arena")
    TArray<FStatusEffectTask> mStatusEffectTasks;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Arena")
    TSubclassOf<AWeapon_Base> mWeaponClass;

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Arena")
    FName mSingleActionAnimationSection = NAME_None;

    UDamageable* mpTargetDamageable = nullptr;

    UStatusEffectsManager* mpTargetStatusEffectsManager = nullptr;

    TArray<AAction_Base*> mAttackingActions;

protected:

    bool CanBeSetupAt_Implementation(const FIntPoint& potentialSetupTile) const override;

    bool TryDistributeAt_Implementation(const FIntPoint& potentialDistributionTile,
                                        int32& outBehaviorAtDistributionTile) override;

    bool FinalizeSetupProcess_Implementation() override;

    void OnCancel_Implementation(AAction_Base* pCancelerAction) override;

    bool OnInterrupt_Implementation(const FIntPoint& interruptionTile,
                                    AAction_Base* pInterruptedBy) override;

    void OnFinishCollisionResolve_Implementation() override;

    void OnStartExecution_Implementation() override;

    void OnStopExecution_Implementation() override;

};