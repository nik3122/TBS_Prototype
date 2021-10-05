

#include "ArenaLobby/PlayerController_ArenaLobby.h"
#include "ArenaLobby/GameMode_ArenaLobby.h"
#include "ArenaLobby/PlayerState_ArenaLobby.h"
#include "ArenaLobby/GameState_ArenaLobby.h"


APlayerController_ArenaLobby::APlayerController_ArenaLobby(const FObjectInitializer& initializer)
    : Super(initializer)
{
    PrimaryActorTick.bCanEverTick = false;
}

void APlayerController_ArenaLobby::InitPlayerState()
{
    Super::InitPlayerState();

    mpPlayerState = Cast<APlayerState_ArenaLobby>(PlayerState);
}

void APlayerController_ArenaLobby::OnRep_PlayerState()
{
    mpPlayerState = Cast<APlayerState_ArenaLobby>(PlayerState);

    Super::OnRep_PlayerState();

    OnRepPlayerState();
}

bool APlayerController_ArenaLobby::Server_RequestBuyArenaUnit_Validate(int32 shopInfoIndex)
{
    return true;
}

void APlayerController_ArenaLobby::Server_RequestBuyArenaUnit_Implementation(int32 unitShopIndex)
{
    mpPlayerState->AddPartyMember(unitShopIndex);
}

bool APlayerController_ArenaLobby::Server_RemovePartyMember_Validate(int32 partyMemberIndex)
{
    return true;
}

void APlayerController_ArenaLobby::Server_RemovePartyMember_Implementation(int32 partyMemberIndex)
{
    mpPlayerState->RemPartyMember(partyMemberIndex);
}

bool APlayerController_ArenaLobby::Server_WriteToChat_Validate(const FString& message)
{
    return true;
}

void APlayerController_ArenaLobby::Server_WriteToChat_Implementation(const FString& message)
{
    auto pGameState = GetWorld()->GetGameState<AGameState_ArenaLobby>();
    FString namedMessage = mpPlayerState->GetPlayerName() + ": " + message;
    pGameState->WriteToChatGlobal(namedMessage);
}

bool APlayerController_ArenaLobby::Server_RequestNameChange_Validate(const FString& newPlayerName)
{
    return true;
}

void APlayerController_ArenaLobby::Server_RequestNameChange_Implementation(const FString& newPlayerName)
{
    auto pGameMode = GetWorld()->GetAuthGameMode<AGameMode_ArenaLobby>();
    pGameMode->ChangeName(this, newPlayerName, true);
}

bool APlayerController_ArenaLobby::Server_SetIsReady_Validate(bool isReady)
{
    return true;
}

bool APlayerController_ArenaLobby::Server_RequestMapChange_Validate(int32 mapIndex)
{
    return true;
}

void APlayerController_ArenaLobby::SetIsReady(bool isReady)
{
    if (isReady &&
        mpPlayerState->GetPartyMembersNum() == 0)
    {
        auto pGameState = GetWorld()->GetGameState<AGameState_ArenaLobby>();
#define LOCTEXT_NAMESPACE "Technical"
        pGameState->WriteToChatLocal("SYSTEM: " + LOCTEXT("ArenaLobbyWarning_EmptyParty", "You must own at least one character.").ToString());
#undef LOCTEXT_NAMESPACE
        return;
    }
    Server_SetIsReady(isReady);
}

void APlayerController_ArenaLobby::Server_RequestMapChange_Implementation(int32 mapIndex)
{
    if (mpPlayerState->mPlayerRole == EPlayerRole::Admin)
    {
        auto pGameState = GetWorld()->GetGameState<AGameState_ArenaLobby>();
        pGameState->SetSelectedMapIndex(mapIndex);
    }
}

void APlayerController_ArenaLobby::Server_SetIsReady_Implementation(bool isReady)
{
    isReady = isReady ? mpPlayerState->GetPartyMembersNum() > 0 : false;
    mpPlayerState->SetIsReady(isReady);
}
