

#include "ArenaLobby/GameState_ArenaLobby.h"
#include "ArenaLobby/ArenaLobbyUtilities.h"
#include "ArenaLobby/PlayerController_ArenaLobby.h"
#include "ArenaLobby/PlayerState_ArenaLobby.h"
#include "Net/UnrealNetwork.h"


void AGameState_ArenaLobby::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(AGameState_ArenaLobby, mConnectedPlayers);
    DOREPLIFETIME(AGameState_ArenaLobby, mMapsList);
    DOREPLIFETIME(AGameState_ArenaLobby, mSelectedMapIndex);
}

void AGameState_ArenaLobby::AddConnectedPlayer(APlayerState_ArenaLobby* pPlayerState)
{
    check(IsValid(pPlayerState));
    mConnectedPlayers.Add(pPlayerState);
    if (GetNetMode() == NM_ListenServer)
    {
        OnRep_ConnectedPlayers();
    }
    ForceNetUpdate();
}

void AGameState_ArenaLobby::RemConnectedPlayer(APlayerState_ArenaLobby* pPlayerState)
{
    check(IsValid(pPlayerState));
    if (mConnectedPlayers.Remove(pPlayerState) != 0)
    {
        if (GetNetMode() == NM_ListenServer)
        {
            OnRep_ConnectedPlayers();
        }
    }
    ForceNetUpdate();
}

const TArray<APlayerState_ArenaLobby*>& AGameState_ArenaLobby::GetConnectedPlayers() const
{
    return mConnectedPlayers;
}

void AGameState_ArenaLobby::WriteToChatGlobal(FString message)
{
    Multicast_WriteToChat(message);
}

void AGameState_ArenaLobby::WriteToChatLocal(FString message)
{
    mChat += message;
    mChat += '\n';
}

void AGameState_ArenaLobby::SetMapsList(TArray<FArenaLobbyMapInfo> mapsList)
{
    mMapsList = mapsList;
    if (GetNetMode() == NM_ListenServer)
    {
        OnRep_MapsList();
    }
    SetSelectedMapIndex(0);
    ForceNetUpdate();
}

const TArray<FArenaLobbyMapInfo>& AGameState_ArenaLobby::GetMapsList() const
{
    return mMapsList;
}

void AGameState_ArenaLobby::SetSelectedMapIndex(int32 selectedMapIndex)
{
    if (mMapsList.IsValidIndex(selectedMapIndex))
    {
        mSelectedMapIndex = selectedMapIndex;
        if (GetNetMode() == NM_ListenServer)
        {
            OnRep_SelectedMapIndex();
        }
        ForceNetUpdate();
    }
}

int32 AGameState_ArenaLobby::GetSelectedMapIndex() const
{
    return mSelectedMapIndex;
}

void AGameState_ArenaLobby::OnRep_ConnectedPlayers()
{
    mEventPlayerListChanged.Broadcast();
}

void AGameState_ArenaLobby::OnRep_MapsList()
{
    mEventMapsListChanged.Broadcast();
}

void AGameState_ArenaLobby::OnRep_SelectedMapIndex()
{
    mEventSelectedMapIndexChanged.Broadcast();
}

void AGameState_ArenaLobby::Multicast_WriteToChat_Implementation(const FString& message)
{
    WriteToChatLocal(message);
}
