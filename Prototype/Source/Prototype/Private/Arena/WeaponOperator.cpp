// Fill out your copyright notice in the Description page of Project Settings.

#include "Arena/WeaponOperator.h"
#include "Arena/ArenaUnit_Interface.h"
#include "Arena/StatusEffectsManager.h"
#include "Arena/Damageable.h"
#include "Arena/AnimationManager.h"


UWeaponOperator::UWeaponOperator()
    : mpWeapon(nullptr)
{
    PrimaryComponentTick.bCanEverTick = false;

}

AWeapon_Base* UWeaponOperator::SpawnAndEquipWeapon(const TSubclassOf<AWeapon_Base>& weaponType)
{
    if (mpWeapon != nullptr &&
        mpWeapon->GetClass() == weaponType)
    {
        return mpWeapon;
    }
    AWeapon_Base* pWeapon = GetWorld()->SpawnActor<AWeapon_Base>(weaponType);
    mIsWeaponSpawned = true;
    EquipWeapon(pWeapon);
    return pWeapon;
}

void UWeaponOperator::EquipWeapon(AWeapon_Base* pWeapon)
{
    UnequipWeapon();

    mpWeapon = pWeapon;
    mIsWeaponSpawned = false;
    if (mpWeapon == nullptr) return;
    
    //	~!~ Assume that owner has animation manager.
    UAnimationManager* pAnimManager = IArenaUnit_Interface::Execute_GetAnimationManager(GetOwner());
    USkeletalMeshComponent* pSkeletalMesh = pAnimManager->GetSkeletalMeshComponent();	
    mpWeapon->AttachToComponent(
        pSkeletalMesh,
        FAttachmentTransformRules::SnapToTargetNotIncludingScale,
        pWeapon->GetAttachmentSocketName()
    );
    mpWeapon->SetWeaponOperator(this);
}

AWeapon_Base* UWeaponOperator::UnequipWeapon()
{
    if (mpWeapon == nullptr) return nullptr;

    if (mIsWeaponSpawned)
    {
        mpWeapon->Destroy();
        return nullptr;
    }

    mpWeapon->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
    mpWeapon->SetWeaponOperator(nullptr);
    return mpWeapon;
}

AWeapon_Base* UWeaponOperator::GetEquipedWeapon() const
{
    return mpWeapon;
}

void UWeaponOperator::SetAttackTargets(const TArray<FWeaponAttackTarget>& targets)
{
    mTargets = targets;
}

void UWeaponOperator::LaunchEquipedWeaponProjectiles()
{
    for (const FWeaponAttackTarget& target : mTargets)
    {
        AProjectile_Base* pProjectile = mpWeapon->LaunchProjectileAtTarget(target);
        pProjectile->mEventHit.BindUObject(this, &UWeaponOperator::OnProjectileHit);
    }
}

void UWeaponOperator::DamageTargetsWithEquipedWeapon()
{
    for (const FWeaponAttackTarget& target : mTargets)
    {
        if (target.mpTargetActor != nullptr)
        {
            DeliverDamage(target.mpTargetActor);
            //mpWeapon->DeliverMeleeDamage(target.mpTargetActor);
        }
    }
}

void UWeaponOperator::SetDamageAndStatusEffects(const FDamageParameters& damage,
                                                const TArray<FStatusEffectTask>& activationTasks)
{
    mDamage = damage;
    mStatusEffectsActivationTasks = activationTasks;
}

void UWeaponOperator::BeginPlay()
{
    Super::BeginPlay();
    
}

void UWeaponOperator::OnProjectileHit(AActor* pHitTarget)
{
    if (pHitTarget != nullptr)
    {
        DeliverDamage(pHitTarget);
    }
}

void UWeaponOperator::DeliverDamage(AActor* pTarget)
{
    /*mEventDeliverDamage.ExecuteIfBound(pTarget);

    if (pTarget->Implements<UArenaUnit_Interface>())
    {
        UDamageable* pDamageable = IArenaUnit_Interface::Execute_GetDamageable(pTarget);
        if (pDamageable != nullptr)
        {
            pDamageable->Damage(mDamage, GetOwner());
        }

        UStatusEffectsManager* pStatusEffectManager = IArenaUnit_Interface::Execute_GetStatusEffectsManager(pTarget);
        if (pStatusEffectManager != nullptr)
        {
            for (const FStatusEffectTask& task : mStatusEffectsActivationTasks)
            {
                pStatusEffectManager->PushStatusEffectTask(task);
            }
        }
    }*/
}
