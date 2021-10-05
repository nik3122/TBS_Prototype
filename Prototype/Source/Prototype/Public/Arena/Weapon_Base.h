// Fill out your copyright notice in the Description page of Project Settings.

#pragma once


#include "CoreMinimal.h"
#include "Math/Vector.h"
#include "Math/Rotator.h"
#include "Projectile_Base.h"
#include "Damage.h"
#include "Damageable.h"
#include "Weapon_Base.generated.h"


class UWeaponOperator;


UENUM(BlueprintType)
enum class EWeaponHandleType: uint8
{
    WeaponHandleType_OneHanded,
    WeaponHandleType_TwoHanded,
    WeaponHandleType_OneHandedInversed,
    WeaponHandleType_TwoHandedInversed,
    WeaponHandleType_InBothHands,
    WeaponHandleType_InBothHandsInversed,
    WeaponHandleType_Bow,
};


UENUM(BlueprintType)
enum class EWeaponType: uint8
{
    WeaponType_Sword,
    WeaponType_Axe,
    WeaponType_Bow,
    WeaponType_Throwable,
};


//
//  ~!~LEGACY~ There is no need for passing all damage and relative parameters tp
//  the projectile as it is just a decoration, so when it hits, it triggers an event.
//  This way WeaponOperator is able to deal a damage to a target.
//
UCLASS(Blueprintable, Abstract)
class PROTOTYPE_API AWeapon_Base :
    public AActor
{
    GENERATED_BODY()

public:

    AWeapon_Base();

    UFUNCTION(BlueprintPure, Category = "Arena")
    UTexture2D* GetIcon() const;
    
    UFUNCTION(BlueprintPure, Category = "Arena")
    FName GetAttachmentSocketName() const;
    
    UFUNCTION(BlueprintPure, Category = "Arena")
    EWeaponHandleType GetHandleType() const;
    
    UFUNCTION(BlueprintPure, BlueprintImplementableEvent, Category = "Arena")
    FVector GetLaunchPosition() const;

    UFUNCTION(BlueprintPure, BlueprintImplementableEvent, Category = "Arena")
    FRotator GetLaunchOrientation() const;
    
    UFUNCTION(BlueprintCallable, Category = "Arena")
    void SetWeaponOperator(UWeaponOperator* pWeaponOperator);

    UFUNCTION(BlueprintCallable, Category = "Arena")
    void LoadProjectile(const TSubclassOf<AProjectile_Base>& projectileType);

    UFUNCTION(BlueprintCallable, Category = "Arena")
    AProjectile_Base* LaunchProjectileAtTarget(const FWeaponAttackTarget& target);

    UFUNCTION(BlueprintCallable, Category = "Arena")
    void DeliverMeleeDamage(AActor* pVictim);

protected:

    UPROPERTY(EditDefaultsOnly, Category = "Arena")
    UTexture2D* mpIcon;

    UPROPERTY(EditDefaultsOnly, Category = "Arena")
    FName mAttachmentSocketName;

    UPROPERTY(EditDefaultsOnly, Category = "Arena")
    EWeaponHandleType mHandleType;

    UPROPERTY(BlueprintReadOnly, Category = "Arena")
    UWeaponOperator* mpWeaponOperator;

    UPROPERTY(BlueprintReadOnly, Category = "Arena")
    TSubclassOf<AProjectile_Base> mLoadedProjectileType;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Arena")
    FDamageParameters mMeleeDamage;
};
