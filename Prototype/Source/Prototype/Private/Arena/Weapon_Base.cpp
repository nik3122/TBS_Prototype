// Fill out your copyright notice in the Description page of Project Settings.

#include "Arena/Weapon_Base.h"
#include "Arena/WeaponOperator.h"
#include "Arena/ArenaUnit_Interface.h"
#include "Arena/CreatureBase.h"
#include "Arena/ActionManager.h"


AWeapon_Base::AWeapon_Base()
{
    PrimaryActorTick.bCanEverTick = false;
    SetCanBeDamaged(false);
    bCollideWhenPlacing = false;
}

UTexture2D* AWeapon_Base::GetIcon() const
{
    return mpIcon;
}

FName AWeapon_Base::GetAttachmentSocketName() const
{
    return mAttachmentSocketName;
}

EWeaponHandleType AWeapon_Base::GetHandleType() const
{
    return mHandleType;
}

void AWeapon_Base::SetWeaponOperator(UWeaponOperator* pWeaponOperator)
{
    mpWeaponOperator = pWeaponOperator;
}

void AWeapon_Base::LoadProjectile(const TSubclassOf<AProjectile_Base>& projectileType)
{
    mLoadedProjectileType = projectileType;
}

AProjectile_Base* AWeapon_Base::LaunchProjectileAtTarget(const FWeaponAttackTarget& target)
{
    AProjectile_Base* pProjectile = GetWorld()->SpawnActor<AProjectile_Base>(mLoadedProjectileType);

    //  ~HACK~ 
    int32 executionPriority = 0;
    if (mpWeaponOperator->GetOwner()->Implements<UArenaUnit_Interface>())
    {
        UActionManager* pActionManager = IArenaUnit_Interface::Execute_GetActionManager(mpWeaponOperator->GetOwner());
        if (pActionManager != nullptr)
        {
            //  Assume that this is only called during a turn execution phase:
            executionPriority = pActionManager->GetExecutionPriority();
        }
    }

    pProjectile->LaunchWithWeaponAttackTarget(GetLaunchPosition(),
                                              target,
                                              mpWeaponOperator->GetOwner(),
                                              executionPriority);
    //  Should we make a projectile just a decoration which won't activate any status effects on the character?
    //  Or should we pass execution priority as parameters during a deep pipeline including weapon operator and weapon?

    return pProjectile;
}

void AWeapon_Base::DeliverMeleeDamage(AActor* pVictim)
{
    if (pVictim->Implements<UArenaUnit_Interface>())
    {
        UDamageable* pDamageable = IArenaUnit_Interface::Execute_GetDamageable(pVictim);
        if (pDamageable != nullptr)
        {
            FDamageParameters deliveredDamage;
            pDamageable->Damage(mMeleeDamage, mpWeaponOperator->GetOwner(), deliveredDamage);
        }
    }
}
