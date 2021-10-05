// Fill out your copyright notice in the Description page of Project Settings.

#include "Arena/Party.h"
#include "Arena/ArenaUtilities.h"
#include "Arena/CreatureBase.h"
#include "Arena/Damageable.h"
#include "Arena/SocialComponent.h"
#include "Net/UnrealNetwork.h"


FLinearColor AParty::sPartyMemberColors[] = {
    FLinearColor(1.f, 1.f, 1.f),
    FLinearColor(1.f, 0.f, 0.f),
    FLinearColor(0.f, 0.85f, 1.f),
    FLinearColor(0.f, 1.f, 0.f),
    FLinearColor(1.f, 0.65f, 0.f),
    FLinearColor(0.5f, 0.f, 1.f),
    FLinearColor(0.35f, 1.f, 0.f),
};


AParty::AParty(const FObjectInitializer& objInit)
    : Super(objInit)
{
    PrimaryActorTick.bCanEverTick = false;

    SetCanBeDamaged(false);
    bCollideWhenPlacing = false;
    SetHidden(true);
    bReplicates = true;
    bAlwaysRelevant = true;
}

void AParty::SetPartyInfo(FArenaPartyInfo partyInfo)
{
    mPartyInfo = partyInfo;
}

const FArenaPartyInfo& AParty::GetPartyInfo() const
{
    return mPartyInfo;
}

const TArray<ACreatureBase*>& AParty::GetMembers()
{
    return mMembers;
}

const TArray<ACreatureBase*>& AParty::GetKilledMembers()
{
    return mKilledMembers;
}

ACreatureBase* AParty::GetMemberByIndex(int32 index)
{
    check(IsIndexValid(index) && "Try to get member by invalid index.");
    check(mMembers[index] != nullptr && "Member by index is invalid.");
    return mMembers[index];
}

ACreatureBase* AParty::TryGetMemberByIndex(int32 index)
{
    if (IsIndexValid(index))
    {
        return mMembers[index];
    }
    return nullptr;
}

int32 AParty::GetPartySize() const
{
    return mMembers.Num();
}

bool AParty::IsNotEmpty() const
{
    return mMembers.Num() > 0;
}

bool AParty::IsIndexValid(int32 index) const
{
    return index >= 0 && index < mMembers.Num();
}

bool AParty::IsInParty(ACreatureBase* pCreature, int32& outMemberIndex)
{
    outMemberIndex = -1;
    if (pCreature == nullptr)
    {
        return false;
    }
    for (int32 memberIndex = 0; memberIndex < mMembers.Num(); ++memberIndex)
    {
        if (pCreature == mMembers[memberIndex])
        {
            outMemberIndex = memberIndex;
            return true;
        }
    }
    return false;
}

bool AParty::GetMemberIndexIfExists(ACreatureBase* pCreature,
                                    int32& outIndex)
{
    check(IsValid(pCreature));
    outIndex = INDEX_NONE;
    if (IsNotEmpty() == false)
    {
        return false;
    }
    mMembers.Find(pCreature, outIndex);
    return outIndex != INDEX_NONE;
}

void AParty::AddMember(ACreatureBase* pCreature)
{
    check(IsValid(pCreature) && "New party member creature must be valid.");
    checkSlow(GetMemberIndexIfExists(pCreature) == false && "Creature already is a member of the party.");
    mMembers.Push(pCreature);
    
    FLinearColor partyMemberColor = FLinearColor::White;
    int32 colorIndex = mMembers.Num() - 1;
    if (colorIndex < 7)
    {
        partyMemberColor = sPartyMemberColors[colorIndex];
    }
    pCreature->SetPartyMemberColor(partyMemberColor);

    pCreature->SetOwner(this);
    pCreature->GetSocialComponent()->SetParty(this);
    pCreature->OnDestroyed.AddDynamic(this, &AParty::OnMemberDestroyed);
    UDamageable* pDamageable = pCreature->GetDamageableComponent();
    check(pDamageable->IsAlive() && "New party member creature must be alive.");
    pDamageable->mEventDeath.AddDynamic(this, &AParty::OnMemberKilled);
    //  ~TODO~ Add reaction to mEventOnRevive.
    UE_LOG(ArenaCommonLog, Log, TEXT("Party \"%s\" added new member %s."), *mPartyInfo.mPartyName, *pCreature->GetName());
    mEventPartyMembersUpdated.Broadcast(this);
}

void AParty::RemoveMemberByIndex(int32 memberIndex)
{
    check(IsIndexValid(memberIndex) && "Member to remove index is invalid.");
    ACreatureBase* pRemovedCreature = mMembers[memberIndex];
    mMembers.RemoveAt(memberIndex);
    
    //  Update existing member's colors:
    for (int32 i = 0; i < mMembers.Num(); ++i)
    {
        ACreatureBase* member = mMembers[i];
        FLinearColor partyMemberColor = FLinearColor::White;
        int32 colorIndex = i;
        if (colorIndex < 7)
        {
            partyMemberColor = sPartyMemberColors[colorIndex];
        }
        member->SetPartyMemberColor(partyMemberColor);
    }

    pRemovedCreature->GetDamageableComponent()->mEventDeath.RemoveDynamic(this, &AParty::OnMemberKilled);
    mEventPartyMembersUpdated.Broadcast(this);
    if (IsPartyDead())
    {
        mEventLastMemberKilled.Broadcast(this, pRemovedCreature);
    }
}

void AParty::RemoveMemeberIfExists(ACreatureBase* pCreature)
{
    if (IsNotEmpty() == false)
    {
        return;
    }
    int32 memberIndex;
    if (GetMemberIndexIfExists(pCreature, memberIndex))
    {
        RemoveMemberByIndex(memberIndex);
    }
}

bool AParty::IsPartyDead() const
{
    for (ACreatureBase* pMember : mMembers)
    {
        if (pMember->GetDamageableComponent()->IsAlive())
        {
            return false;
        }
    }
    return true;
}

void AParty::BeginPlay()
{
    Super::BeginPlay();
    
    //
}

void AParty::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(AParty, mMembers);
    DOREPLIFETIME(AParty, mKilledMembers);
    DOREPLIFETIME(AParty, mPartyInfo);
}

void AParty::OnMemberKilled(UDamageable* pDamageable)
{
    ACreatureBase* pCreature = Cast<ACreatureBase>(pDamageable->GetOwner());
    check(IsValid(pCreature) && "AParty::OnMemberKilled() must be called only for creature.");
    int32 memberIndex = INDEX_NONE;
    GetMemberIndexIfExists(pCreature, memberIndex);
    check(memberIndex != INDEX_NONE && "AParty::OnMemberKilled() must be called only for existing party member.");
    mMembers.RemoveAt(memberIndex);
    mKilledMembers.Push(pCreature);
    UE_LOG(ArenaCommonLog, Verbose, TEXT("Party \"%s\" member %s was killed."), *mPartyInfo.mPartyName, *pCreature->GetName());
    mEventPartyMembersUpdated.Broadcast(this);
    if (IsPartyDead())
    {
        mEventLastMemberKilled.Broadcast(this, pCreature);
    }
}

void AParty::OnMemberDestroyed(AActor* pDestroyedActor)
{
    ACreatureBase* pDestroyedCreature = Cast<ACreatureBase>(pDestroyedActor);
    check(IsValid(pDestroyedCreature) && "AParty is tracking non-creature actor!");
    RemoveMemeberIfExists(pDestroyedCreature);
}

void AParty::OnRep_Members()
{
    for (const ACreatureBase* pCreature : mMembers)
    {
        if (pCreature != nullptr)
        {
            pCreature->GetSocialComponent()->SetParty(this);
        }
    }
    mEventPartyMembersUpdated.Broadcast(this);
}