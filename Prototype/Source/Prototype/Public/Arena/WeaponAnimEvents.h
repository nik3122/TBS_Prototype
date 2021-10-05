
#pragma once


#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "WeaponAnimEvents.generated.h"


/**

*/
UCLASS(Blueprintable, const)
class PROTOTYPE_API ULaunchEquipedWeaponProjectile
	: public UAnimNotify
{
	GENERATED_BODY()

public:
	
	virtual void Notify(USkeletalMeshComponent* pSkeletalMeshComp,
						UAnimSequenceBase* pAnimation) override;
};



/**

*/
UCLASS(Blueprintable, const)
class PROTOTYPE_API UDamageWithEquipedWeapon
	: public UAnimNotify
{
	GENERATED_BODY()

public:

	virtual void Notify(USkeletalMeshComponent* pSkeletalMeshComp,
						UAnimSequenceBase* pAnimation) override;
};