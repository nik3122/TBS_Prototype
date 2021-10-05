
#include "Arena/WeaponAnimEvents.h"
#include "Arena/ArenaUnit_Interface.h"
#include "Arena/WeaponOperator.h"
#include "UnknownCommon.h"


void ULaunchEquipedWeaponProjectile::Notify(USkeletalMeshComponent* pSkeletalMeshComp,
                                            UAnimSequenceBase* pAnimation)
{
    AActor* pOwner = pSkeletalMeshComp->GetOwner();
    if (IsValid(pOwner))
    {
        TScriptInterface<IArenaUnit_Interface> arenaUnitInterface(pOwner);
        if (arenaUnitInterface)
        {
            UWeaponOperator* pWeaponOperator = arenaUnitInterface->Execute_GetWeaponOperator(arenaUnitInterface.GetObject());
            pWeaponOperator->LaunchEquipedWeaponProjectiles();
        }
#if !UE_BUILD_SHIPPING
        else
        {
            UE_LOG(UnknownCommonLog, Warning, TEXT("%s doesn't implement ArenaUnit_Interface. GenericAnimNotify can't be processed."), *(pOwner->GetClass()->GetName()));
        }
#endif
    }
}

void UDamageWithEquipedWeapon::Notify(USkeletalMeshComponent* pSkeletalMeshComp,
                                      UAnimSequenceBase* pAnimation)
{
    return;
    AActor* pOwner = pSkeletalMeshComp->GetOwner();
    TScriptInterface<IArenaUnit_Interface> arenaUnitInterface(pOwner);
    if (arenaUnitInterface)
    {
        UWeaponOperator* pWeaponOperator = arenaUnitInterface->Execute_GetWeaponOperator(arenaUnitInterface.GetObject());
        pWeaponOperator->DamageTargetsWithEquipedWeapon();
    }
#if !UE_BUILD_SHIPPING
    else
    {
        UE_LOG(UnknownCommonLog, Warning, TEXT("%s doesn't implement ArenaUnit_Interface. GenericAnimNotify can't be processed."), *(pOwner->GetClass()->GetName()));
    }
#endif
}
