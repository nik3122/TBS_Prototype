#pragma once


#include "ArenaUnitShopInfo.h"
#include "PlayerController_ArenaLobby.generated.h"


class APlayerState_ArenaLobby;


UCLASS(Blueprintable)
class APlayerController_ArenaLobby
    : public APlayerController
{
    GENERATED_BODY()

public:

    APlayerController_ArenaLobby(const FObjectInitializer& initializer = FObjectInitializer::Get());

    void InitPlayerState() override;

    void OnRep_PlayerState() override;

    APlayerState_ArenaLobby* GetPlayerStateArenaLobby() const { return mpPlayerState; }

    UFUNCTION(Server, Reliable, WithValidation, BlueprintCallable, Category = "ArenaLobby")
    void Server_RequestBuyArenaUnit(int32 unitShopIndex);

    UFUNCTION(Server, Reliable, WithValidation, BlueprintCallable, Category = "ArenaLobby")
    void Server_RemovePartyMember(int32 partyMemberIndex);

    UFUNCTION(Server, Reliable, WithValidation, BlueprintCallable, Category = "ArenaLobby")
    void Server_WriteToChat(const FString& message);

    UFUNCTION(Server, Reliable, WithValidation, BlueprintCallable, Category = "ArenaLobby")
    void Server_RequestNameChange(const FString& newPlayerName);

    UFUNCTION(Server, Reliable, WithValidation, BlueprintCallable, Category = "ArenaLobby")
    void Server_RequestMapChange(int32 mapIndex);

    UFUNCTION(BlueprintCallable, Category = "ArenaLobby")
    void SetIsReady(bool isReady);

protected:

    UPROPERTY(BlueprintReadOnly, Category = "ArenaLobby")
    APlayerState_ArenaLobby* mpPlayerState = nullptr;

protected:

    UFUNCTION(BlueprintImplementableEvent, Category = "ArenaLobby")
    void OnRepPlayerState();

    UFUNCTION(Server, Reliable, WithValidation)
    void Server_SetIsReady(bool isReady);

};