

#include "ArenaLobby/GameMode_ArenaLobby.h"
#include "ArenaLobby/GameSession_ArenaLobby.h"
#include "ArenaLobby/GameState_ArenaLobby.h"
#include "ArenaLobby/PlayerController_ArenaLobby.h"
#include "ArenaLobby/PlayerState_ArenaLobby.h"
#include "GameInstance_Unknown.h"
#include "Engine/DataTable.h"


AGameMode_ArenaLobby::AGameMode_ArenaLobby(const FObjectInitializer& objectInitializer)
    : Super(objectInitializer)
{
#define LOCTEXT_NAMESPACE "Technical"
    GameSessionClass = AGameSession_ArenaLobby::StaticClass();
    GameStateClass = AGameState_ArenaLobby::StaticClass();
    PlayerControllerClass = APlayerController_ArenaLobby::StaticClass();
    PlayerStateClass = APlayerState_ArenaLobby::StaticClass();
    DefaultPawnClass = 0;
    DefaultPlayerName = LOCTEXT("DefaultPlayerName", "Unknown");
    HUDClass = 0;
#undef LOCTEXT_NAMESPACE
}

void AGameMode_ArenaLobby::InitGameState()
{
    Super::InitGameState();

    mpGameState = GetGameState<AGameState_ArenaLobby>();
    if (mpGameState != nullptr)
    {
        if (mpMapsCollection != nullptr)
        {
            TArray<FArenaLobbyMapInfo*> rows;
            mpMapsCollection->GetAllRows<FArenaLobbyMapInfo>(TEXT("AGameMode_ArenaLobby::InitGameState()"), rows);
            TArray<FArenaLobbyMapInfo> mapsList;
            for (auto* pMapInfo : rows)
            {
                mapsList.Push(*pMapInfo);
            }
            mpGameState->SetMapsList(mapsList);
        }

        mpGameState->mEventMapsListChanged.AddDynamic(this, &AGameMode_ArenaLobby::ResetPlayersReadiness);
        mpGameState->mEventSelectedMapIndexChanged.AddDynamic(this, &AGameMode_ArenaLobby::ResetPlayersReadiness);
    }
}

void AGameMode_ArenaLobby::PreLogin(const FString& options, const FString& address, const FUniqueNetIdRepl& uniqueId, FString& errorMessage)
{
    if (mpAdmin != nullptr &&
        mpClient != nullptr)
    {
        errorMessage = "Session is full.";
        return;
    }

    Super::PreLogin(options, address, uniqueId, errorMessage);
}

FString AGameMode_ArenaLobby::InitNewPlayer(APlayerController* pNewPlayerController, const FUniqueNetIdRepl& uniqueId, const FString& options, const FString& portal)
{
    FString errorMessage = Super::InitNewPlayer(pNewPlayerController, uniqueId, options, portal);
    if (errorMessage.IsEmpty())
    {
        ChangeName(pNewPlayerController, "Unknown", false);
    }
    return errorMessage;
}

void AGameMode_ArenaLobby::PostLogin(APlayerController* pNewPlayer)
{
    Super::PostLogin(pNewPlayer);
    if (mpAdmin == nullptr)
    {
        mpAdmin = Cast<APlayerController_ArenaLobby>(pNewPlayer);
        InitPlayer(mpAdmin);
    }
    else
    {
        mpClient = Cast<APlayerController_ArenaLobby>(pNewPlayer);
        InitPlayer(mpClient);
    }
}

void AGameMode_ArenaLobby::Logout(AController* pExiting)
{
    auto pExitingPlayer = Cast<APlayerController_ArenaLobby>(pExiting);
    mpGameState->RemConnectedPlayer(pExitingPlayer->GetPlayerStateArenaLobby());
    //  TODO: If the match is about to start - cancel it.
    ResetPlayersReadiness();

    mpAdmin = pExiting == mpAdmin ? nullptr : mpAdmin;
    mpClient = pExiting == mpClient ? nullptr : mpClient;

    Super::Logout(pExiting);
}

void AGameMode_ArenaLobby::ResetPlayersReadiness()
{
    if (IsValid(mpAdmin))
    {
        auto pPlayerState = mpAdmin->GetPlayerState<APlayerState_ArenaLobby>();
        if (IsValid(pPlayerState))
        {
            pPlayerState->SetIsReady(false);
        }
    }
    if (IsValid(mpClient))
    {
        auto pPlayerState = mpClient->GetPlayerState<APlayerState_ArenaLobby>();
        if (IsValid(pPlayerState))
        {
            pPlayerState->SetIsReady(false);
        }
    }
}

void AGameMode_ArenaLobby::InitPlayer(APlayerController_ArenaLobby* pPlayerController)
{
    check(pPlayerController != nullptr);
    auto* pPlayerState = pPlayerController->GetPlayerState<APlayerState_ArenaLobby>();
    check(pPlayerState != nullptr);
    bool isAdmin = pPlayerController == mpAdmin;
    pPlayerState->mPartyColor = isAdmin ? FLinearColor(0.8f, 0.0f, 0.0f) : FLinearColor(0.0f, 0.2f, 0.8f);
    pPlayerState->mPlayerRole = isAdmin ? EPlayerRole::Admin : EPlayerRole::Client;
    pPlayerState->mMoney = mPlayersMoney;
    //  ~!~ Put it into config?
    pPlayerState->mMaxPartySize = 5;

    TArray<FArenaUnitShopInfo*> unitsList;
    mpUnitsCollection->GetAllRows<FArenaUnitShopInfo>(TEXT("AGameMode_ArenaLobby::InitPlayerState()"), unitsList);
    TArray<FArenaUnitShopInfo> unitsShop;
    for (FArenaUnitShopInfo* pUnitInfo : unitsList)
    {
        unitsShop.Push(*pUnitInfo);
    }
    pPlayerState->SetUnitsShop(unitsShop);

    mpGameState->AddConnectedPlayer(pPlayerState);
    pPlayerState->mEventIsReadyChanged.AddDynamic(this, &AGameMode_ArenaLobby::OnPlayerIsReadyChanged);
    pPlayerState->mEventPartyMembersChanged.AddDynamic(this, &AGameMode_ArenaLobby::ResetPlayersReadiness);
}

void AGameMode_ArenaLobby::OnPlayerIsReadyChanged()
{
    if (IsValid(mpAdmin) && IsValid(mpClient))
    {
        auto pAdminState = mpAdmin->GetPlayerState<APlayerState_ArenaLobby>();
        auto pClientState = mpClient->GetPlayerState<APlayerState_ArenaLobby>();
        if (IsValid(pAdminState) && IsValid(pClientState) &&
            pAdminState->GetIsReady() && pClientState->GetIsReady())
        {
            //  Run Arena with the appropriate settings:
            auto pGameInstance = GetGameInstance<UGameInstance_Unknown>();
            pGameInstance->mArenaSetupInfo.mArenaGameMode = EArenaGameMode::PvP;
            //  Party one:
            pGameInstance->mArenaSetupInfo.mPartyOneName = pAdminState->GetPlayerName();
            pGameInstance->mArenaSetupInfo.mPartyOneColor = pAdminState->mPartyColor;
            pGameInstance->mArenaSetupInfo.mPartyOneMembers = pAdminState->GetPartyMembersClasses();
            //  Party two:
            pGameInstance->mArenaSetupInfo.mPartyTwoName = pClientState->GetPlayerName();
            pGameInstance->mArenaSetupInfo.mPartyTwoColor = pClientState->mPartyColor;
            pGameInstance->mArenaSetupInfo.mPartyTwoMembers = pClientState->GetPartyMembersClasses();
            pGameInstance->OpenArena(mpGameState->GetMapsList()[mpGameState->GetSelectedMapIndex()].mLevelName);
        }
    }
}

#if WITH_EDITOR

void AGameMode_ArenaLobby::PostEditChangeProperty(FPropertyChangedEvent& propertyChangedEvent)
{
    FName propertyName = propertyChangedEvent.MemberProperty != nullptr ? propertyChangedEvent.MemberProperty->GetFName() : NAME_None;
    if (propertyName == GET_MEMBER_NAME_CHECKED(AGameMode_ArenaLobby, mpUnitsCollection))
    {
        if (mpUnitsCollection != nullptr &&
            mpUnitsCollection->RowStructName != "ArenaUnitShopInfo")
        {
            mpUnitsCollection = nullptr;
        }
    }
    else if (propertyName == GET_MEMBER_NAME_CHECKED(AGameMode_ArenaLobby, mpMapsCollection))
    {
        if (mpMapsCollection != nullptr &&
            mpMapsCollection->RowStructName != "ArenaLobbyMapInfo")
        {
            mpMapsCollection = nullptr;
        }
    }
    else
    {
        Super::PostEditChangeProperty(propertyChangedEvent);
    }
}

#endif // #if WITH_EDITOR