#pragma once


#include "CoreMinimal.h"
#include "Arena/Action_SeparateMovement_Base.h"
#include "Arena/Damage.h"
#include "Arena/StatusEffectsManager.h"
#include "Action_SeparateMovement_Attack.generated.h"


class AWeapon_Base;


UCLASS()
class AAction_SeparateMovement_Attack
    : public AAction_SeparateMovement_Base
{
    GENERATED_BODY()

public:

    AAction_SeparateMovement_Attack(const FObjectInitializer& initializer = FObjectInitializer::Get());

    void Tick(float deltaSeconds) override;

    void OnStartExecution_Implementation() override;

    void OnStopExecution_Implementation() override;

protected:

    struct FVictimInfo
    {
        TWeakObjectPtr<AActor> mArenaUnit;
        FIntPoint mTile;
    };

protected:

    TArray<FVictimInfo> mVictims;

    UPROPERTY(EditDefaultsOnly, Category = "Action", Meta = (DisplayName = "WeaponClass"))
    TSubclassOf<AWeapon_Base> mWeaponClass;

    UPROPERTY(EditDefaultsOnly, Category = "Action", Meta = (DisplayName = "StatusEffectTasks"))
    TArray<FStatusEffectTask> mStatusEffectTasks;

    UPROPERTY(EditDefaultsOnly, Category = "Action", Meta = (DisplayName = "Damage"))
    FDamageParameters mDamage;

    //  Name of single action animation used to display an attack on the executor.
    UPROPERTY(EditDefaultsOnly, Category = "Action", Meta = (DisplayName = "AttackAnimationName"))
    FName mAttackAnimation;

    //  When processing victims ignore high and low obstacles on direct visibility check.
    UPROPERTY(EditDefaultsOnly, Category = "Action", Meta = (DisplayName = "IgnoreHighAndLowObstacle"))
    bool mIgnoreHighAndLowObstacle = false;

    //  When processing victims ignore only low obstacles on direct visibility check.
    UPROPERTY(EditDefaultsOnly, Category = "Action", Meta = (DisplayName = "IgnoreLowObstacle"))
    bool mIgnoreLowObstacle = false;

protected:

    void ApplyInner() override;

    //  It is called for each potential victim within the pattern at the final position
    //  to determine whether a victim should be affected.
    //  Return true to accept potential victim, othervise return false.
    virtual bool ProcessPotentialVictim(const FVictimInfo& victimInfo);

    virtual void PrioritizeVictims();

    virtual void AffectVictims();

    UFUNCTION()
    void OnAttackAnimationFinished();
};