// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "GenericAnimNotify.generated.h"


UENUM(BlueprintType)
enum class EGenericAnimNotify: uint8
{
	EGenericAnimNotify_SingleActionAnimFinished,				//	Is triggered when any Single Action Animation (or anim sequence) is finished.
	EGenericAnimNotify_AnimationTransfer,						//	
	EGenericAnimNotify_WeaponGeneric,							//	
	EGenericAnimNotify_SpellCastGeneric,						//	
};

/**
 * 
 */
UCLASS(Blueprintable, const)
class PROTOTYPE_API UGenericAnimNotify : public UAnimNotify
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, Category = "Unknown|Animation")
	EGenericAnimNotify mType;

public:
	
	virtual void Notify(USkeletalMeshComponent* pSkeletalMeshComp,
						UAnimSequenceBase* pAnimation) override;

};
