#pragma once


#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "GameMode_ArenaLobby.generated.h"


class AGameState_ArenaLobby;
class APlayerController_ArenaLobby;
class APlayerState_ArenaLobby;
class UDataTable;


UCLASS()
class AGameMode_ArenaLobby
    : public AGameModeBase
{
    GENERATED_BODY()

public:

    AGameMode_ArenaLobby(const FObjectInitializer& objectInitializer);

    //void InitGame(const FString& mapName, const FString& options, FString& errorMessage) override;

    void InitGameState() override;

    void PreLogin(const FString& options, const FString& address, const FUniqueNetIdRepl& uniqueId, FString& errorMessage) override;

    FString InitNewPlayer(APlayerController* pNewPlayerController, const FUniqueNetIdRepl& uniqueId, const FString& options, const FString& portal = TEXT("")) override;

    void PostLogin(APlayerController* pNewPlayer) override;

    void Logout(AController* pExiting) override;

    UFUNCTION(BlueprintCallable, Category = "ArenaLobby")
    void ResetPlayersReadiness();

protected:

    UPROPERTY()
    APlayerController_ArenaLobby* mpAdmin = nullptr;

    UPROPERTY()
    APlayerController_ArenaLobby* mpClient = nullptr;

    UPROPERTY()
    AGameState_ArenaLobby* mpGameState = nullptr;

    //  Collection of the available units in shop. Raw type: FArenaUnitShopInfo.
    UPROPERTY(EditDefaultsOnly, Category = "ArenaLobby")
    UDataTable* mpUnitsCollection = nullptr;

    //  Collection of the available arena maps. Raw type: FArenaLobbyMapInfo.
    UPROPERTY(EditDefaultsOnly, Category = "ArenaLobby")
    UDataTable* mpMapsCollection = nullptr;

    UPROPERTY(EditDefaultsOnly, Category = "ArenaLobby")
    int32 mPlayersMoney = 1000;

    UPROPERTY(EditDefaultsOnly, Category = "ArenaLobby")
    int32 mPartyMaxSize = 5;

protected:

    void InitPlayer(APlayerController_ArenaLobby* pPlayer);

    UFUNCTION()
    void OnPlayerIsReadyChanged();

#if WITH_EDITOR

    void PostEditChangeProperty(FPropertyChangedEvent& propertyChangedEvent) override;

#endif // #if WITH_EDITOR
};