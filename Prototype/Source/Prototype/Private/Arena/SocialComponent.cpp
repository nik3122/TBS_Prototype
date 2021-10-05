// Fill out your copyright notice in the Description page of Project Settings.

#include "Arena/SocialComponent.h"
#include "Arena/ArenaUtilities.h"
#include "Arena/Party.h"
#include "Arena/CreatureBase.h"
#include "Arena/ArenaManager_Interface.h"
#include "Net/UnrealNetwork.h"


USocialComponent::USocialComponent():
    mpParty(nullptr)
{
	PrimaryComponentTick.bCanEverTick = false;

}

void USocialComponent::SetParty(AParty* pParty)
{
    mpParty = pParty;
}

AParty* USocialComponent::GetParty() const
{
    return mpParty;
}

AParty* USocialComponent::GetEnemyParty() const
{
    TScriptInterface<IArenaManager_Interface> arenaManagerInterface;
    UArenaUtilities::GetArenaManagerInterface(GetOwner(), arenaManagerInterface);
    AParty* pAIParty = IArenaManager_Interface::Execute_GetAIParty(arenaManagerInterface.GetObject());
    AParty* pPlayerParty = IArenaManager_Interface::Execute_GetPlayerParty(arenaManagerInterface.GetObject());
    return mpParty == pAIParty ? pPlayerParty : pAIParty;
}

void USocialComponent::GetAllies(TArray<ACreatureBase*>& allyCreatures) const
{
    allyCreatures = GetParty()->GetMembers();
}

void USocialComponent::GetEnemies(TArray<ACreatureBase*>& enemyCreatures) const
{
    enemyCreatures = GetEnemyParty()->GetMembers();
}

void USocialComponent::BeginPlay()
{
    Super::BeginPlay();

}

void USocialComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(USocialComponent, mpParty);
}

