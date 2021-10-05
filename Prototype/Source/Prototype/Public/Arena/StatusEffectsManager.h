// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Arena/StatusEffect.h"
#include "Version.h"
#include "StatusEffectsManager.generated.h"


class UDamageable;
class UBattleLogItem_Base;


/*
 *  Helper structure which is used for both activation and deactivation of the effects.
 *  For deactivation mDuration <= 0.
 */
USTRUCT(BlueprintType)
struct FStatusEffectTask
{
    GENERATED_BODY()

public:

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "StatusEffect", Meta = (DisplayName = "Effect Class"))
    TSubclassOf<AStatusEffect> mEffectClass;

    /**
     *  Set mDuration > 0 to activate the specified status effect.
     *  Set mDuration <= 0 to deactivate the specified status effect.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "StatusEffect", Meta = (DisplayName = "Duration (turns)"))
    int32 mDuration = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "StatusEffect", Meta = (DisplayName = "Stage"))
    int32 mStage = 0;

    /**
     *  Task priority. Used when using UStatusEffectsManager::PushStatusEffectTask().
     */
    UPROPERTY(BlueprintReadWrite, Category = "StatusEffect", Meta = (DisplayName = "Task Priority"))
    int32 mPriority = 0;

    //  (optional) Battle log item where the result of the task would be written.
    UPROPERTY(BlueprintReadWrite, Category = "StatusEffect")
    UBattleLogItem_Base* mpResultBattleLogItem = nullptr;
};


struct State_StatusEffectsManager
    : public State_Base
{
    DECLARE_CLASS_VERSION(1, 0);
    State_StatusEffect** mppStatusEffectsStates;
    int32 mStatusEffectsCount;
};


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PROTOTYPE_API UStatusEffectsManager
    : public UActorComponent
    , public ArenaStateObject_Base<State_StatusEffectsManager>
{
    CHECK_CLASS_VERSION_EQUAL(State_StatusEffectsManager, 1, 0);

    GENERATED_BODY()

public:

    DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FEventStatusEffectActivated, AActor*, pEffectOwner, AStatusEffect*, pActivatedStatusEffect);
    UPROPERTY(BlueprintAssignable, Category = "StatusEffect")
    FEventStatusEffectActivated mEventStatusEffectActivated;

    DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FEventStatusEffectDeactivated, AActor*, pEffectOwner, AStatusEffect*, pDeactivatedStatusEffect);
    UPROPERTY(BlueprintAssignable, Category = "StatusEffect")
    FEventStatusEffectDeactivated mEventStatusEffectDeactivated;

public:

    // Sets default values for this component's properties
    UStatusEffectsManager(const FObjectInitializer& objectInitializer = FObjectInitializer::Get());

    void Activate(bool reset = false) override;

    void Deactivate() override;

    //  Pushes task into sorted (by priority) task queue which will be executed on turn finalization.
    UFUNCTION(BlueprintCallable, Category = "StatusEffect")
    void PushStatusEffectTask(const FStatusEffectTask& task);

    //  Executes task in place.
    UFUNCTION(BlueprintCallable, Category = "StatusEffect")
    void ExecuteStatusEffectTask(const FStatusEffectTask& task);

    UFUNCTION(BlueprintCallable, Category = "StatusEffect")
    void ActivateStatusEffect(TSubclassOf<AStatusEffect> effectClass, int32 duration, int32 stage);

    UFUNCTION(BlueprintCallable, Category = "StatusEffect")
    void DeactivateStatusEffect(TSubclassOf<AStatusEffect> effectClass);

    UFUNCTION(BlueprintCallable, Category = "StatusEffect")
    bool IsStatusEffectActive(TSubclassOf<AStatusEffect> effectClass);

    UFUNCTION(BlueprintPure, Category = "StatusEffect")
    const TArray<AStatusEffect*>& GetAllEffects() const;

    UFUNCTION(BlueprintPure, Category = "StatusEffect", meta = (BlueprintAutocast))
    AStatusEffect* GetEffectByType(TSubclassOf<AStatusEffect> effectClass);

    UFUNCTION(BlueprintPure, Category = "StatusEffect")
    TArray<AStatusEffect*> GetActiveEffects();

protected:

    UPROPERTY()
    TArray<AStatusEffect*> mEffects;

    UPROPERTY(EditDefaultsOnly, Category = "StatusEffect", Meta = (DisplayName = "Supported Status Effects"))
    TArray<TSubclassOf<AStatusEffect>> mSupportedStatusEffects;

protected:

    // Called when the game starts
    virtual void BeginPlay() override;

    virtual void EndPlay(EEndPlayReason::Type reason) override;

    UFUNCTION()
    void OnSystemsSetup();

    UFUNCTION()
    void ApplyStatusEffectsBeforeActions();

    UFUNCTION()
    void ApplyStatusEffectsAfterActions();

    UFUNCTION()
    void OnFinalizeTurn();

    UFUNCTION()
    void OnEffectActivated(AStatusEffect* pStatusEffect);

    UFUNCTION()
    void OnEffectDeactivated(AStatusEffect* pStatusEffect);

    void StoreState_Inner(ArenaStateContext& context, State_StatusEffectsManager& state) const override;

    bool RestoreState_Inner(const State_StatusEffectsManager& state) override;

private:

    TArray<FStatusEffectTask> mStatusEffectTasks;

    FDelegateHandle mDelegateApplyStatusEffectsBeforeActions;

    FDelegateHandle mDelegateApplyStatusEffectsAfterActions;

private:

    void ExecuteTasks();
};
