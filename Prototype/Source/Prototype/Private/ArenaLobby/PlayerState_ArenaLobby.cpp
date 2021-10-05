

#include "ArenaLobby/PlayerState_ArenaLobby.h"
#include "ArenaLobby/ArenaLobbyUtilities.h"
#include "Arena/PlayerState_Arena.h"
#include "Net/UnrealNetwork.h"


void APlayerState_ArenaLobby::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(APlayerState_ArenaLobby, mPlayerRole);
    DOREPLIFETIME(APlayerState_ArenaLobby, mPartyColor);
    DOREPLIFETIME(APlayerState_ArenaLobby, mMoney);
    DOREPLIFETIME(APlayerState_ArenaLobby, mUnitsShop);
    DOREPLIFETIME(APlayerState_ArenaLobby, mPartyMembersShopIndices);
    DOREPLIFETIME(APlayerState_ArenaLobby, mIsReady);
}

void APlayerState_ArenaLobby::SetUnitsShop(TArray<FArenaUnitShopInfo> unitsShop)
{
    mUnitsShop = unitsShop;
    if (GetNetMode() == NM_ListenServer)
    {
        OnRep_UnitsShop();
    }
    ForceNetUpdate();
}

void APlayerState_ArenaLobby::AddPartyMember(int32 unitShopIndex)
{
    if (mUnitsShop.IsValidIndex(unitShopIndex) &&
        mPartyMembersShopIndices.Num() < mMaxPartySize)
    {
        FArenaUnitShopInfo& shopInfo = mUnitsShop[unitShopIndex];
        if (shopInfo.mCount < shopInfo.mMaxCount &&
            shopInfo.mCost <= mMoney)
        {
            mPartyMembersShopIndices.Add(unitShopIndex);
            mMoney -= shopInfo.mCost;
            ++shopInfo.mCount;
            if (GetNetMode() == NM_ListenServer)
            {
                OnRep_PartyMembers();
                OnRep_UnitsShop();
            }
            ForceNetUpdate();
        }
    }
}

void APlayerState_ArenaLobby::RemPartyMember(int32 partyMemberIndex)
{
    if (mPartyMembersShopIndices.IsValidIndex(partyMemberIndex))
    {
        int32 remUnitShopIndex = mPartyMembersShopIndices[partyMemberIndex];
        mPartyMembersShopIndices.RemoveAt(partyMemberIndex);
        FArenaUnitShopInfo& shopInfo = mUnitsShop[remUnitShopIndex];
        --shopInfo.mCount;
        mMoney += shopInfo.mCost;
        check(mUnitsShop[remUnitShopIndex].mCount >= 0);
        if (GetNetMode() == NM_ListenServer)
        {
            OnRep_PartyMembers();
            OnRep_UnitsShop();
        }
        ForceNetUpdate();
    }
}

int32 APlayerState_ArenaLobby::GetPartyMembersNum() const
{
    return mPartyMembersShopIndices.Num();
}

TArray<TSubclassOf<ACreatureBase>> APlayerState_ArenaLobby::GetPartyMembersClasses() const
{
    TArray<TSubclassOf<ACreatureBase>> partyMembersClasses;
    for (auto unitShopIndex : mPartyMembersShopIndices)
    {
        partyMembersClasses.Push(mUnitsShop[unitShopIndex].mCreatureClass);
    }
    return partyMembersClasses;
}

void APlayerState_ArenaLobby::SetIsReady(bool isReady)
{
    mIsReady = isReady;
    if (GetNetMode() == NM_ListenServer)
    {
        OnRep_IsReady();
    }
    ForceNetUpdate();
}

void APlayerState_ArenaLobby::OnRep_PlayerName()
{
    Super::OnRep_PlayerName();

    mEventPlayerNameChanged.Broadcast();
}

void APlayerState_ArenaLobby::OnRep_UnitsShop()
{
    mEventUnitsShopChanged.Broadcast();
}

void APlayerState_ArenaLobby::OnRep_PartyMembers()
{
    mEventPartyMembersChanged.Broadcast();
}

void APlayerState_ArenaLobby::OnRep_IsReady()
{
    mEventIsReadyChanged.Broadcast();
}
