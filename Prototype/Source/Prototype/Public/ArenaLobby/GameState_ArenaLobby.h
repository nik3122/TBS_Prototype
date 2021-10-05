#pragma once


#include "CoreMinimal.h"
#include "ArenaLobbyMapInfo.h"
#include "GameFramework/GameStateBase.h"
#include "GameState_ArenaLobby.generated.h"


class APlayerState_ArenaLobby;
class APlayerController_ArenaLobby;


UCLASS()
class AGameState_ArenaLobby
    : public AGameStateBase
{
    GENERATED_BODY()

public:

    DECLARE_DYNAMIC_MULTICAST_DELEGATE(FEventGameStateChanged);

    UPROPERTY(BlueprintAssignable, Category = "ArenaLobby")
    FEventGameStateChanged mEventPlayerListChanged;

    UPROPERTY(BlueprintAssignable, Category = "ArenaLobby")
    FEventGameStateChanged mEventMapsListChanged;

    UPROPERTY(BlueprintAssignable, Category = "ArenaLobby")
    FEventGameStateChanged mEventSelectedMapIndexChanged;

public:

    void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const;

    void AddConnectedPlayer(APlayerState_ArenaLobby* pPlayerState);

    void RemConnectedPlayer(APlayerState_ArenaLobby* pPlayerState);

    UFUNCTION(BlueprintPure, Category = "ArenaLobby")
    const TArray<APlayerState_ArenaLobby*>& GetConnectedPlayers() const;

    void WriteToChatGlobal(FString message);

    void WriteToChatLocal(FString message);

    void SetMapsList(TArray<FArenaLobbyMapInfo> mapsList);

    UFUNCTION(BlueprintPure, Category = "ArenaLobby")
    const TArray<FArenaLobbyMapInfo>& GetMapsList() const;

    void SetSelectedMapIndex(int32 selectedMapIndex);

    UFUNCTION(BlueprintPure, Category = "ArenaLobby")
    int32 GetSelectedMapIndex() const;

protected:

    UPROPERTY(ReplicatedUsing = OnRep_ConnectedPlayers, BlueprintReadWrite, Category = "ArenaLobby")
    TArray<APlayerState_ArenaLobby*> mConnectedPlayers;

    //  ~!~ Instead of OnRep event it is better to cyclically check if its not empty to initialize UI.
    UPROPERTY(ReplicatedUsing = OnRep_MapsList, BlueprintReadWrite, Category = "ArenaLobby")
    TArray<FArenaLobbyMapInfo> mMapsList;

    UPROPERTY(ReplicatedUsing = OnRep_SelectedMapIndex, BlueprintReadWrite, Category = "ArenaLobby")
    int32 mSelectedMapIndex = 0;

    //  Chat is local for each user, so it's not replicated:
    UPROPERTY(BlueprintReadOnly, Category = "ArenaLobby")
    FString mChat;

protected:

    UFUNCTION()
    void OnRep_ConnectedPlayers();

    UFUNCTION()
    void OnRep_MapsList();

    UFUNCTION()
    void OnRep_SelectedMapIndex();

    UFUNCTION(NetMulticast, Reliable)
    void Multicast_WriteToChat(const FString& message);
};