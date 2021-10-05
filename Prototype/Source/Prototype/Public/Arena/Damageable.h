// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Damage.h"
#include "ArenaState.h"
#include "Version.h"
#include "Damageable.generated.h"


class UDamageable;
class UBattleLogItem_Base;


struct State_Damageable
    : public State_Base
{
    DECLARE_CLASS_VERSION(1, 0);
    int32 mMaxHP;
    int32 mCurrentHP;
    FDamageParameters mResistClear;
    FDamageParameters mResistBonus;
    FDamageParameters mDefenceClear;
    FDamageParameters mDefenceBonus;
    bool mIsImmortal;
};


UENUM(BlueprintType)
enum class EDamageableTaskType : uint8
{
    Heal,
    Damage,
    AddResistBonus,
    AddDefenceBonus,
};


USTRUCT(BlueprintType)
struct FDamageableTask
{
    GENERATED_BODY()

public:

    //  Type of the task.
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Damageable")
    EDamageableTaskType mTaskType;

    //  Value used as damage, defence or resistance bonus.
    //  If mTaskType == EDamageableTaskType::Heal only first field is used.
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Damageable")
    FDamageParameters mValue;

    //  Initiator of the task.
    UPROPERTY(BlueprintReadWrite, Category = "Damageable")
    AActor* mpInitiator = nullptr;

    //  Priority of the task.
    UPROPERTY(BlueprintReadWrite, Category = "Damageable")
    int32 mPriority = 0;

    //  (optional) Battle log item where the result of the task would be written.
    UPROPERTY(BlueprintReadWrite, Category = "Damageable")
    UBattleLogItem_Base* mpResultBattleLogItem = nullptr;
};


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PROTOTYPE_API UDamageable
    : public UActorComponent
    , public ArenaStateObject_Base<State_Damageable>
{
    CHECK_CLASS_VERSION_EQUAL(State_Damageable, 1, 0);

    GENERATED_BODY()

public:

    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FEventDeath, UDamageable*, pDamageable);
    UPROPERTY(BlueprintAssignable, Category = "Damageable")
    FEventDeath mEventDeath;

    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FEventRevive, UDamageable*, pDamageable);
    UPROPERTY(BlueprintAssignable, Category = "Damageable")
    FEventRevive mEventRevive;

    DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FEventHPChanged, UDamageable*, pDamageable, int32, deltaHP, AActor*, dealer);
    UPROPERTY(BlueprintAssignable, Category = "Damageable")
    FEventHPChanged mEventHPChanged;

    UPROPERTY(BlueprintAssignable, Category = "Damageable")
    FEventHPChanged mEventVisualHPChanged;

public:

    UDamageable(const FObjectInitializer& objectInitializer = FObjectInitializer::Get());

    void DuplicateState(const UDamageable* source);

    //  Pushes damageable task into stack which is reasolved on turn finalization.
    UFUNCTION(BlueprintCallable, Category = "Damageable", meta=(DeprecatedFunction))
    void PushDamageableTask(const FDamageableTask& damageableTask);

    //  Executes damageable task in place.
    UFUNCTION(BlueprintCallable, Category = "Damageable")
    void ExecuteDamageableTask(const FDamageableTask& damageableTask);

    /*
        Returns HP change.
    */
    UFUNCTION(BlueprintCallable, Category = "Damageable")
    int32 Damage(const FDamageParameters& damage,
                 AActor* pDamageDealer,
                 FDamageParameters& outDeliveredDamage);

    UFUNCTION(BlueprintCallable, Category = "Damageable")
    void Heal(int32 hp, AActor* pDealer);

    UFUNCTION(BlueprintPure, Category = "Damageable")
    bool IsAlive() const;

    UFUNCTION(BlueprintPure, Category = "Damageable")
    int32 GetNominalMaxHP() const;

    UFUNCTION(BlueprintPure, Category = "Damageable")
    int32 GetCurrentMaxHP() const;

    UFUNCTION(BlueprintPure, Category = "Damageable")
    int32 GetCurrentHP() const;

    UFUNCTION(BlueprintPure, Category = "Damageable")
    const FDamageParameters& GetResistClear() const;

    UFUNCTION(BlueprintCallable, Category = "Damageable")
    void AddResistBonus(const FDamageParameters& addBonus);

    UFUNCTION(BlueprintCallable, Category = "Damageable")
    void SetResistBonus(const FDamageParameters& newBonus);

    UFUNCTION(BlueprintPure, Category = "Damageable")
    const FDamageParameters& GetResistBonus() const;

    UFUNCTION(BlueprintPure, Category = "Damageable")
    FDamageParameters GetResistFull() const;

    UFUNCTION(BlueprintPure, Category = "Damageable")
    const FDamageParameters& GetDefenceClear() const;

    UFUNCTION(BlueprintCallable, Category = "Damageable")
    void AddDefenceBonus(const FDamageParameters& addBonus);

    UFUNCTION(BlueprintCallable, Category = "Damageable")
    void SetDefenceBonus(const FDamageParameters& newBonus);

    UFUNCTION(BlueprintPure, Category = "Damageable")
    const FDamageParameters& GetDefenceBonus() const;

    UFUNCTION(BlueprintPure, Category = "Damageable")
    FDamageParameters GetDefenceFull() const;

    UFUNCTION(BlueprintCallable, Category = "Damageable")
    void SetImmortal(bool isImmortal);

    UFUNCTION(BlueprintCallable, Category = "Damageable")
    bool IsImmortal() const;

    UFUNCTION(BlueprintCallable, Category = "Damageable")
    void Revive(int32 hpAfterRevive);

protected:

    //  Nominal maximum level of HP that creature can contain.
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Damageable")
    int32 mMaxHP;

    //  Nominal resistance creature parameters. Value 10 corresponds 1%, 1000 corresponds 100%.
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Damageable")
    FDamageParameters mResistClear;

    //  Nominal defence creature parameters.
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Damageable")
    FDamageParameters mDefenceClear;

protected:

    // Called when the game starts
    virtual void BeginPlay() override;

    void SetHP(int32 newHP);

    UFUNCTION()
    void OnSystemsSetup();

    UFUNCTION()
    void OnUpdateDamageable();

    void StoreState_Inner(ArenaStateContext& arenaState, State_Damageable& state) const override;

    bool RestoreState_Inner(const State_Damageable& state) override;

private:

    FDamageParameters mResistBonus;

    FDamageParameters mDefenceBonus;

    //  Current level of HP.
    int32 mCurrentHP;

    int32 mVisualHP;

    //  Flag presenting immortality (true = is immortal).
    bool mIsImmortal;

    TArray<FDamageableTask> mDamageableTasks;

    bool mShouldExecuteTasks = false;

    bool mNeedDispatchDeathEvent = false;

private:

    void ExecuteTasks();
};
