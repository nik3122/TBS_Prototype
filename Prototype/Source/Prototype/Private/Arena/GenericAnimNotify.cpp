// Fill out your copyright notice in the Description page of Project Settings.

#include "Arena/GenericAnimNotify.h"
#include "UnknownCommon.h"
#include "Arena/AnimationManager.h"
#include "Arena/ArenaUnit_Interface.h"


void UGenericAnimNotify::Notify(USkeletalMeshComponent* pSkeletalMeshComp,
								UAnimSequenceBase* pAnimation)
{
	AActor* pOwner = pSkeletalMeshComp->GetOwner();
	TScriptInterface<IArenaUnit_Interface> arenaUnitInterface(pOwner);
	if (arenaUnitInterface)
	{
		UAnimationManager* pAnimationManager = arenaUnitInterface->Execute_GetAnimationManager(arenaUnitInterface.GetObject());
		pAnimationManager->OnGenericAnimNotify(mType);
	}
#if !UE_BUILD_SHIPPING
	else
	{
		UE_LOG(UnknownCommonLog, Warning, TEXT("%s doesn't implement ArenaUnit_Interface. GenericAnimNotify can't be processed."), *(pOwner->GetClass()->GetName()));
	}
#endif
}
