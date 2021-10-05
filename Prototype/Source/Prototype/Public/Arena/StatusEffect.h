// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Arena/ArenaState.h"
#include "Version.h"
#include "StatusEffect.generated.h"


class UStatusEffectsManager;
class AStatusEffect;


struct State_StatusEffect
    : public State_Base
{
    DECLARE_CLASS_VERSION(1, 1);
    const ANSICHAR* mpClassName;
    int32 mClassNameSize;
    int32 mTurnsTimer;
};


/**
 *  Defines the moment during the game loop when status effect is applied.
 */
UENUM(BlueprintType)
enum class ETimeOfApplication : uint8
{
    BeforeActions,
    AfterActions,
    BeforeAndAfterActions,
};



UCLASS()
class PROTOTYPE_API AStatusEffect 
    : public AActor
    , public ArenaStateObject_Base<State_StatusEffect>
{
    CHECK_CLASS_VERSION_EQUAL(State_StatusEffect, 1, 1);

    GENERATED_BODY()

public:

    DECLARE_DELEGATE_OneParam(FEventStatusEffectActivated, AStatusEffect*);
    FEventStatusEffectActivated mEventActivated;

    DECLARE_DELEGATE_OneParam(FEventStatusEffectDeactivated, AStatusEffect*);
    FEventStatusEffectDeactivated mEventDeactivated;

public:

    AStatusEffect(const FObjectInitializer& objectInitializer = FObjectInitializer::Get());

    virtual void Setup(UStatusEffectsManager* pStatusEffectsManager,
                       AActor* pTargetActor);

    virtual void Apply(ETimeOfApplication timeOfApplication);

    virtual void Activate(int32 duration,
                          int32 stage);

    virtual void Deactivate();

    void OnTurnFinished();

    UStatusEffectsManager* GetStatusEffectManager() const;

    UFUNCTION(BlueprintPure, Category = "StatusEffect")
    UTexture2D* GetIcon() const;

    UFUNCTION(BlueprintPure, Category = "StatusEffect")
    ETimeOfApplication GetTimeOfApplication() const;

    UFUNCTION(BlueprintPure, Category = "StatusEffect")
    bool IsActive() const;

    UFUNCTION(BlueprintPure, Category = "StatusEffect")
    int32 GetDuration() const;

    UFUNCTION(BlueprintPure, Category = "StatusEffect")
    int32 GetStage() const;

    UFUNCTION(BlueprintPure, Category = "StatusEffect")
    const FText& GetEffectName() const;

    UFUNCTION(BlueprintPure, Category = "StatusEffect")
    const FText& GetFullDescription() const;

protected:

    UPROPERTY(EditDefaultsOnly, Category = "StatusEffect", Meta = (DisplayName = "Icon"))
    UTexture2D* mpIcon = nullptr;

    UPROPERTY(EditDefaultsOnly, Category = "StatusEffect", Meta = (DisplayName = "Time Of Application"))
    ETimeOfApplication mTimeOfApplication = ETimeOfApplication::BeforeAndAfterActions;

    UPROPERTY(BlueprintReadOnly, Category = "StatusEffect", Meta = (DisplayName = "Target Actor"))
    AActor* mpTargetActor = nullptr;

    UPROPERTY(BlueprintReadWrite, Category = "StatusEffect", Meta = (DisplayName = "Turns Timer"))
    int32 mTurnsTimer = 0;

    UPROPERTY(BlueprintReadOnly, Category = "StatusEffect", Meta = (DisplayName = "Stage"))
    int32 mStage = 0;

    UPROPERTY(EditDefaultsOnly, Category = "StatusEffect", Meta = (DisplayName = "Status Effect Name"))
    FText mName;

    UPROPERTY(EditDefaultsOnly, Category = "StatusEffect", Meta = (DisplayName = "Full Description"))
    FText mFullDescription;

    UPROPERTY()
    UStatusEffectsManager* mpStatusEffectManager = nullptr;

protected:

    UFUNCTION(BlueprintImplementableEvent, Category = "StatusEffect")
    void Setup_Inner();

    UFUNCTION(BlueprintImplementableEvent, Category = "StatusEffect")
    void Activate_Inner(int32 stage);

    UFUNCTION(BlueprintImplementableEvent, Category = "StatusEffect")
    void Reactivate_Inner(int32 duration,
                          int32 stage,
                          int32& newDuration,
                          int32& newStage);

    UFUNCTION(BlueprintImplementableEvent, Category = "StatusEffect")
    void Deactivate_Inner();

    UFUNCTION(BlueprintImplementableEvent, Category = "StatusEffect")
    void Apply_Inner(ETimeOfApplication timeOfApplication);

    void StoreState_Inner(ArenaStateContext& context, State_StatusEffect& state) const override;

    bool RestoreState_Inner(const State_StatusEffect& state) override;

};
