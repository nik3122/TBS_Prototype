#pragma once


#include "CoreMinimal.h"
#include "Arena/Action_SeparateMovement_Base.h"
#include "Action_SeparateMovement_Heal.generated.h"


UCLASS()
class AAction_SeparateMovement_Heal
    : public AAction_SeparateMovement_Base
{
    GENERATED_BODY()

public:

    AAction_SeparateMovement_Heal(const FObjectInitializer& initializer = FObjectInitializer::Get());

    void Tick(float deltaSeconds) override;

    void OnStartExecution_Implementation() override;

    void OnStopExecution_Implementation() override;

protected:

    struct FTargetUnitInfo
    {
        TWeakObjectPtr<AActor> mArenaUnit;
        FIntPoint mTile;
    };

protected:

    TArray<FTargetUnitInfo> mTargetUnits;

    UPROPERTY(EditDefaultsOnly, Category = "Action", Meta = (DisplayName = "Heal HP Amount"))
    int32 mHealHP = 0;

    //  Name of single action animation used to display an attack on the executor.
    UPROPERTY(EditDefaultsOnly, Category = "Action", Meta = (DisplayName = "Single Action Animation"))
    FName mActionAnimation = NAME_None;

    //  Max number of target units to be affected.
    UPROPERTY(EditDefaultsOnly, Category = "Action", Meta = (DisplayName = "Max Target Units Number"))
    int32 mMaxTargetUnitsNumber = 1;

protected:

    void ApplyInner() override;

    //  It is called for each potential target unit within the pattern at the final position
    //  to determine whether an arena unit should be affected.
    //  Return true to accept potential victim, othervise return false.
    virtual bool ProcessPotentialTargetUnit(const FTargetUnitInfo& targetUnitInfo);

    //  Sort mTargetUnits in the order they will be affected.
    virtual void PrioritizeTargetUnits();

    virtual void AffectTargetUnits();

    UFUNCTION()
    void OnActionAnimationFinished();
};