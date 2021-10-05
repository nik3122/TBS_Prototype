// Fill out your copyright notice in the Description page of Project Settings.

#pragma once


#include "CoreMinimal.h"
#include "Weapon_Base.h"
#include "StatusEffectsManager.h"
#include "WeaponOperator.generated.h"


UCLASS()
class PROTOTYPE_API UWeaponOperator
    : public UActorComponent
{
    GENERATED_BODY()
    
public:

    DECLARE_DELEGATE_OneParam(FEventDeliverDamage, AActor*);
    FEventDeliverDamage mEventDeliverDamage;

public:

    UWeaponOperator();

    UFUNCTION(BlueprintCallable, Category = "Arena")
    AWeapon_Base* SpawnAndEquipWeapon(const TSubclassOf<AWeapon_Base>& weaponType);

    UFUNCTION(BlueprintCallable, Category = "Arena")
    void EquipWeapon(AWeapon_Base* pWeapon);

    UFUNCTION(BlueprintCallable, Category = "Arena")
    AWeapon_Base* UnequipWeapon();

    UFUNCTION(BlueprintPure, Category = "Arena")
    AWeapon_Base* GetEquipedWeapon() const;

    //	If put LoadProjectile proxy function here it will be possible to play load animations.

    UFUNCTION(BlueprintCallable, Category = "Arena")
    void SetAttackTargets(const TArray<FWeaponAttackTarget>& targets);

    UFUNCTION(BlueprintCallable, Category = "Arena")
    void LaunchEquipedWeaponProjectiles();

    UFUNCTION(BlueprintCallable, Category = "Arena")
    void DamageTargetsWithEquipedWeapon();

    //  ~HACK~ Aiming not to use multicast delegates for damage delivery this function should be used:
    UFUNCTION(BlueprintCallable, Category = "Arena")
    void SetDamageAndStatusEffects(const FDamageParameters& damage,
                                   const TArray<FStatusEffectTask>& activationTasks);

protected:

    UPROPERTY(BlueprintReadOnly, Category = "Arena")
    AWeapon_Base* mpWeapon;

    UPROPERTY(BlueprintReadOnly, Category = "Arena")
    TArray<FWeaponAttackTarget> mTargets;

    UPROPERTY()
    bool mIsWeaponSpawned;

    FDamageParameters mDamage;

    TArray<FStatusEffectTask> mStatusEffectsActivationTasks;

protected:
    
    virtual void BeginPlay() override;

    void DeliverDamage(AActor* pTarget);

    UFUNCTION()
    void OnProjectileHit(AActor* pHitTarget);
};
