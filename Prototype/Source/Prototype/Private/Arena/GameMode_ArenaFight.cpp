// Fill out your copyright notice in the Description page of Project Settings.

#include "Arena/GameMode_ArenaFight.h"
#include "Arena/ArenaUtilities.h"
#include "Arena/PlayerController_ArenaFight.h"
#include "Arena/GameState_Arena.h"
#include "Arena/PlayerState_Arena.h"
#include "Arena/ArenaManager.h"
#include "Arena/PassMap.h"
#include "Arena/FireSystem.h"
#include "Arena/ActionMap.h"
#include "Arena/TurnsManager.h"
#include "Arena/Party.h"
#include "Arena/Controller_ArenaAI.h"
#include "Arena/Weather.h"
#include "GameInstance_Unknown.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/DateTime.h"
#include "WorldSettings_Unknown.h"
#include "UnknownCommon.h"


AGameMode_ArenaFight::AGameMode_ArenaFight()
{
    GameStateClass = AGameState_Arena::StaticClass();
    PlayerStateClass = APlayerState_Arena::StaticClass();
    PlayerControllerClass = APlayerController_ArenaFight::StaticClass();

    //  No execution phase is needed on dedicated as it is only visuals:
    mSkipExecution = GetNetMode() == ENetMode::NM_DedicatedServer;
}

EArenaGameMode AGameMode_ArenaFight::GetArenaGameMode() const
{
    return mArenaGameMode;
}

void AGameMode_ArenaFight::OnPlayerArenaSetup(APlayerController* pPlayer)
{
    UE_LOG(ArenaCommonLog, Log, TEXT("Player '%s' finished arena setup."), *pPlayer->PlayerState->GetPlayerName());
    if (pPlayer == mpPlayerOne)
    {
        mIsArenaSetupPlayerOne = true;
    }
    if (pPlayer == mpPlayerTwo)
    {
        mIsArenaSetupPlayerTwo = true;
    }
    TryBeginBattle();
}

void AGameMode_ArenaFight::OnPlayerPlanningFinished(APlayerController* pPlayer)
{
    UE_LOG(ArenaCommonLog, Log, TEXT("Player '%s' finished planning."), *pPlayer->PlayerState->GetPlayerName());
    if (IsWaitingForPlayersFinishPlanning())
    {
        if (IsValid(mpPlayerOne) && mpPlayerOne->GetArenaPlayerState()->mIsPlanningFinished &&
            (IsValid(mpPlayerTwo) == false || mpPlayerTwo->GetArenaPlayerState()->mIsPlanningFinished))
        {
            UE_LOG(ArenaCommonLog, Verbose, TEXT("All players finished planning. "));

            mpPlayerOne->GetArenaPlayerState()->mIsPlanningFinished = false;
            if (IsValid(mpPlayerTwo))
            {
                mpPlayerTwo->GetArenaPlayerState()->mIsPlanningFinished = false;
            }

            mIsWaitingForPlayersFinishPlanning = false;
            mIsWaitingForPlayersFinishExecution = true;

            UArenaUtilities::GetTurnsManager(this)->FinishPlanning();
            AIMakeDecision();
            UArenaUtilities::GetActionMap(this)->StartActionSubmitting();
        }
    }
    else
    {
        UE_LOG(ArenaCommonLog, Warning, TEXT("NOT EXPECTED players finish planning now."));
    }
}

void AGameMode_ArenaFight::OnPlayerExecutionFinished(APlayerController* pPlayer)
{
    UE_LOG(ArenaCommonLog, Log, TEXT("Player '%s' finished execution."), *pPlayer->PlayerState->GetPlayerName());
    if (IsWaitingForPlayersFinishExecution())
    {
        StartNewTurnOrFinishBattle();
    }
    else
    {
        UE_LOG(ArenaCommonLog, Warning, TEXT("NOT EXPECTED players finish execution now."));
    }
}

void AGameMode_ArenaFight::OnPlayerEoARetry(APlayerController* pPlayer)
{
    UE_LOG(ArenaCommonLog, Log, TEXT("Player '%s' wants to retry."), *pPlayer->PlayerState->GetPlayerName());
    if (IsWaitingForPlayersEoADecision())
    {
        if (IsValid(mpPlayerOne) && mpPlayerOne->GetArenaPlayerState()->mEoAWantsToRetry &&
            IsValid(mpPlayerTwo) && mpPlayerTwo->GetArenaPlayerState()->mEoAWantsToRetry)
        {
            mpPlayerOne->GetArenaPlayerState()->mEoAWantsToRetry = false;
            mpPlayerTwo->GetArenaPlayerState()->mEoAWantsToRetry = false;
            GetWorld()->ServerTravel(GetGameInstance<UGameInstance_Unknown>()->mArenaLobbyLevel.ToString());
        }
    }
    else
    {
        UE_LOG(ArenaCommonLog, Warning, TEXT("NOT EXPECTED players make EoA decision now."));
    }
}

void AGameMode_ArenaFight::LeaveArena()
{
    //  ~!~?~TODO~ Leave the game depending wether it is a MP/SP quick battle session or MP/SP global map.

    //UGameplayStatics::GetPlayerController(this, 0)->ConsoleCommand("quit");

    auto pGameInstance = UUnknownCommon::GetGameInstanceUnknown(this);
    // ~!~?~ Move global map states into C++ as it is impossible to get any of them from here.
    if (pGameInstance->mAfterArenaLevelName.IsEmpty())
    {
#if WITH_EDITOR
        //  ~!~TODO~ If PIE - leave PIE.
        UGameplayStatics::GetPlayerController(this, 0)->ConsoleCommand("quit");
#endif // #if WITH_EDITOR
    }
    else
    {
        FName nextLevelName(*pGameInstance->mAfterArenaLevelName);
        UGameplayStatics::OpenLevel(this, nextLevelName);
    }
}

void AGameMode_ArenaFight::StartPlay()
{
    Super::StartPlay();
}

void AGameMode_ArenaFight::InitGame(const FString& mapName,
                                    const FString& options,
                                    FString& errorMessage)
{
    DetermineArenaGameMode();
    check(GetArenaGameMode() != EArenaGameMode::INVALID && "Wasn't able to determine ArenaGameMode.");

    Super::InitGame(mapName,
                    options,
                    errorMessage);

    FActorSpawnParameters spawnParam;
    //spawnParam.Owner = this;
    spawnParam.bNoFail = true;
}

void AGameMode_ArenaFight::InitGameState()
{
    Super::InitGameState();

    mpArenaGameState = Cast<AGameState_Arena>(GameState);
    mpArenaGameState->mArenaGameMode = GetArenaGameMode();
}

void AGameMode_ArenaFight::BeginPlay()
{
    Super::BeginPlay();

    if (UArenaUtilities::AreSystemsSetup(this))
    {
        OnSystemsSetup();
    }
    else
    {
        UArenaUtilities::GetArenaManager(this)->mEventSystemsSetup.AddDynamic(this, &AGameMode_ArenaFight::OnSystemsSetup);
    }

    TryBeginBattle();
}

void AGameMode_ArenaFight::SpawnAndSetupParties()
{
    UE_LOG(ArenaCommonLog, Verbose, TEXT("Spawn and setup parties."));

    auto& arenaSetupInfo = UUnknownCommon::GetGameInstanceUnknown(this)->mArenaSetupInfo;

    AParty* pPartyOne = nullptr;
    AParty* pPartyTwo = nullptr;
    {
        FActorSpawnParameters spawnParams;
        spawnParams.Name = FName("PartyOne");
        spawnParams.Owner = mpPlayerOne;
        pPartyOne = GetWorld()->SpawnActor<AParty>(spawnParams);

        spawnParams.Name = FName("PartyTwo");
        //  Doesn't matter whether PlayerTwo is nullptr or not. Anyway PlayerOne shouldn't own it.
        spawnParams.Owner = GetArenaGameMode() == EArenaGameMode::HotSeat ? mpPlayerOne : mpPlayerTwo;
        pPartyTwo = GetWorld()->SpawnActor<AParty>(spawnParams);

        mpArenaGameState->GetArenaManager()->SetParties(pPartyOne, pPartyTwo);
    }

    pPartyOne->SetPartyInfo({ arenaSetupInfo.mPartyOneName, arenaSetupInfo.mPartyOneColor });
    pPartyTwo->SetPartyInfo({ arenaSetupInfo.mPartyTwoName, arenaSetupInfo.mPartyTwoColor });

    //  Possess parties according to arena setup info:
    mpPlayerOne->Possess(pPartyOne);
    switch (GetArenaGameMode())
    {
    case EArenaGameMode::PvE:
        {
            FActorSpawnParameters spawnParams;
            spawnParams.Owner = this;
            spawnParams.bNoFail = true;
            mpControllerPartyAI = GetWorld()->SpawnActor<AController_ArenaAI>(spawnParams);
            mpControllerPartyAI->Possess(pPartyTwo);
            mpControllerPartyAI->SetAIConfig(UUnknownCommon::GetGameInstanceUnknown(this)->mArenaAIConfig);
        }
        break;

    case EArenaGameMode::PvP:
        {
            check(IsValid(mpPlayerTwo));
            mpPlayerTwo->Possess(pPartyTwo);
        }
        break;
    }
    
    //  Collect all members placed directly on the level:
    {
        TArray<AActor*> partyOnePlacedMembers;
        UGameplayStatics::GetAllActorsWithTag(this, "player", partyOnePlacedMembers);
        for (AActor* pMember : partyOnePlacedMembers)
        {
            auto pCreature = Cast<ACreatureBase>(pMember);
            if (IsValid(pCreature))
            {
                pPartyOne->AddMember(pCreature);
            }
        }

        TArray<AActor*> partyTwoPlacedMembers;
        UGameplayStatics::GetAllActorsWithTag(this, "ai", partyTwoPlacedMembers);
        for (AActor* pMember : partyTwoPlacedMembers)
        {
            auto pCreature = Cast<ACreatureBase>(pMember);
            if (IsValid(pCreature))
            {
                pPartyTwo->AddMember(pCreature);
            }
        }
    }

    APassMap* pPassMap = UArenaUtilities::GetPassMap(this);
    TArray<FIntPoint> spawnTilesPlayerParty;
    TArray<FIntPoint> spawnTilesAIParty;
    pPassMap->GetSpawnTiles(spawnTilesPlayerParty, spawnTilesAIParty);

    FVector passMapCenter;
    FVector passMapExtent;
    pPassMap->GetActorBounds(false, passMapCenter, passMapExtent);

    UGameInstance_Unknown* pGameInstance = Cast<UGameInstance_Unknown>(GetGameInstance());
    check(IsValid(pGameInstance) && "Game instance must be of GameInstance_Unknown type!");
    TArray<TSubclassOf<ACreatureBase>>& playerPartyMembers = arenaSetupInfo.mPartyOneMembers;
    TArray<TSubclassOf<ACreatureBase>>& AIPartyMembers = arenaSetupInfo.mPartyTwoMembers;

    checkf(spawnTilesPlayerParty.Num() >= playerPartyMembers.Num(), TEXT("Not enough spawn tiles (%u) for player party (%u)!"), spawnTilesPlayerParty.Num(), playerPartyMembers.Num());
    for (int32 i = 0; i < playerPartyMembers.Num(); ++i)
    {
        FVector spawnPosition = pPassMap->GetTilePositionWorld(spawnTilesPlayerParty[i]);
        FVector viewVector = passMapCenter - spawnPosition;
        viewVector.Z = 0.0f;
        FRotator spawnRotation = viewVector.ToOrientationRotator();
        ACreatureBase* pSpawnedCreature = GetWorld()->SpawnActor<ACreatureBase>(playerPartyMembers[i], spawnPosition, spawnRotation);
        pPartyOne->AddMember(pSpawnedCreature);
    }

    checkf(spawnTilesAIParty.Num() >= AIPartyMembers.Num(), TEXT("Not enough spawn tiles (%u) for AI party (%u)!"), spawnTilesPlayerParty.Num(), playerPartyMembers.Num());
    for (int32 i = 0; i < AIPartyMembers.Num(); ++i)
    {
        FVector spawnPosition = pPassMap->GetTilePositionWorld(spawnTilesAIParty[i]);
        FVector viewVector = passMapCenter - spawnPosition;
        viewVector.Z = 0.0f;
        FRotator spawnRotation = viewVector.ToOrientationRotator();
        ACreatureBase* pSpawnedCreature = GetWorld()->SpawnActor<ACreatureBase>(AIPartyMembers[i], spawnPosition, spawnRotation);
        pPartyTwo->AddMember(pSpawnedCreature);
    }
}

void AGameMode_ArenaFight::PreLogin(const FString& options, const FString& address, const FUniqueNetIdRepl& uniqueId, FString& errorMessage)
{
    Super::PreLogin(options, address, uniqueId, errorMessage);

    if (IsValid(mpPlayerOne) &&
        (IsValid(mpPlayerTwo) || GetArenaGameMode() != EArenaGameMode::PvP))
    {
        errorMessage = "Arena session is full.";
    }
}

void AGameMode_ArenaFight::GenericPlayerInitialization(AController* pController)
{
    Super::GenericPlayerInitialization(pController);

    auto pNewArenaPlayer = Cast<APlayerController_ArenaFight>(pController);
    if (IsValid(pNewArenaPlayer))
    {
        //  ~TODO~ Introduce ArenaSetupInfo and detect whether it is mpPlayerOne or mpPlayerTwo.
        if (mpPlayerOne == nullptr)
        {
            UE_LOG(ArenaCommonLog, Log, TEXT("'%s' connected as player ONE"), *pNewArenaPlayer->PlayerState->GetPlayerName());
            mpPlayerOne = pNewArenaPlayer;
            mpArenaGameState->mpPlayerStateOne = mpPlayerOne->GetArenaPlayerState();
            check(mpArenaGameState->mpPlayerStateOne != nullptr && "Arena Player State must be valid for PlayerOne.");
        }
        else if (mpPlayerTwo == nullptr)
        {
            UE_LOG(ArenaCommonLog, Log, TEXT("'%s' connected as player TWO"), *pNewArenaPlayer->PlayerState->GetPlayerName());
            mpPlayerTwo = pNewArenaPlayer;
            mpArenaGameState->mpPlayerStateTwo = mpPlayerTwo->GetArenaPlayerState();
            check(mpArenaGameState->mpPlayerStateTwo != nullptr && "Arena Player State must be valid for PlayerTwo.");
        }
        else
        {
            check(false && "Arena doesn't support more than 2 arena players.");
        }
        TryBeginBattle();
    }
}

bool AGameMode_ArenaFight::ArePlayersReady() const
{
    bool isPlayerOneReady = IsValid(mpPlayerOne) && mIsArenaSetupPlayerOne;
    bool isPlayerTwoReady = IsValid(mpPlayerTwo) && mIsArenaSetupPlayerTwo;
    auto& arenaSetupInfo = UUnknownCommon::GetGameInstanceUnknown(this)->mArenaSetupInfo;
    return (GetArenaGameMode() != EArenaGameMode::PvP && isPlayerOneReady) ||
        (GetArenaGameMode() == EArenaGameMode::PvP && isPlayerOneReady && isPlayerTwoReady);
}

void AGameMode_ArenaFight::TryBeginBattle()
{
    if (HasActorBegunPlay() &&
        ArePlayersReady() &&
        UArenaUtilities::AreSystemsSetup(this) &&
        HasMatchStarted() == false)
    {
        UE_LOG(ArenaCommonLog, Log, TEXT("GameMode BEGIN BATTLE"));
        SpawnAndSetupParties();
        mpArenaGameState->mIsBattleStarted = true;
        mIsWaitingForPlayersFinishPlanning = true;

        //  ~!~TEMP~FIXIT~ Battle shouldn't be done at the moment when parties are just spawned!
        FTimerHandle timer;
        auto StartBattle = [this]() {
            UArenaUtilities::GetTurnsManager(this)->StartPlanning();
        };
        GetWorld()->GetTimerManager().SetTimer(timer,
                                               StartBattle,
                                               5.f, false);
    }
}

void AGameMode_ArenaFight::Logout(AController* exiting)
{
    UE_LOG(ArenaCommonLog, Log, TEXT("'%s' left the game."), *exiting->PlayerState->GetPlayerName());
    mpPlayerOne = mpPlayerOne == exiting ? nullptr : mpPlayerOne;
    mpPlayerTwo = mpPlayerTwo == exiting ? nullptr : mpPlayerTwo;
    //  If left during EoA - left player must have the only option - return to main menu.
    if (IsWaitingForPlayersEoADecision())
    {
        switch (GetArenaGameMode())
        {
        case EArenaGameMode::PvP:
            //  ~TOFO~ Notify staying player about other player has left. Disable retry button for staying player.
            break;

        case EArenaGameMode::PvE:
            //  ~TODO~ Online PvE may be played online only in combination with the global map.
            //  In the end of PvE arena on global map arena server must reconnect player to the global map server.
            break;

        case EArenaGameMode::HotSeat:
            UE_LOG(ArenaCommonLog, Error, TEXT("Hot Seat arena session must be played exceptionally offline."));
            break;
        }
    }
    //  If left before EoA:
    else
    {
        switch (GetArenaGameMode())
        {
        case EArenaGameMode::PvP:
            //  ~TODO~ Notify left player about EoA. Set him winning.
            {
                APlayerController_ArenaFight* pLeftPlayer = mpPlayerOne == exiting ? mpPlayerTwo : mpPlayerOne;
                if (IsValid(pLeftPlayer))
                {
                    pLeftPlayer->Client_NotifyOpponentQuit();
                }
            }
            break;

        case EArenaGameMode::PvE:
            //  ~TODO~ Online PvE may be played online only in combination with the global map.
            //  If player is disconnected during the arena on the global map - arena server must notify global map server about player disconnection.
            //  Global map server may want to save the state of the global map before arena started to allow recreate the session again and to load
            //  from the last synched point.
            break;

        case EArenaGameMode::HotSeat:
            UE_LOG(ArenaCommonLog, Error, TEXT("Hot Seat arena session must be played exceptionally offline."));
            break;
        }
    }
}

void AGameMode_ArenaFight::OnSystemsSetup()
{
    ATurnsManager* pTurnsManager = UArenaUtilities::GetTurnsManager(this);
    pTurnsManager->SetSkipExecution(mSkipExecution);
    pTurnsManager->mEventTurnFinished.BindUObject(this, &AGameMode_ArenaFight::OnTurnFinished);

    TryBeginBattle();
}

void AGameMode_ArenaFight::OnTurnFinished()
{
    UE_LOG(ArenaCommonLog, VeryVerbose, TEXT("Server finished turn's execution."));
    mIsServerExecutionFinished = true;
    if (GetNetMode() == NM_ListenServer ||
        GetNetMode() == NM_Standalone)
    {
        check(IsValid(mpPlayerOne) && "PlayerOne must be always valid on ListenServer");
        mpPlayerOne->GetArenaPlayerState()->mIsExecutionFinished = true;
    }
    StartNewTurnOrFinishBattle();
}

void AGameMode_ArenaFight::AIMakeDecision()
{
    if (IsValid(mpControllerPartyAI))
    {
        mpControllerPartyAI->AIMakeDecision();
    }
}

void AGameMode_ArenaFight::StartNewTurnOrFinishBattle()
{
    if (TryFinishBattle() == false)
    {
        TryStartNewTurn();
    }
}

bool AGameMode_ArenaFight::TryFinishBattle()
{
    AArenaManager* pArenaManager = UArenaUtilities::GetArenaManager(this);
    AParty* pPartyOne = pArenaManager->GetPartyOne_Implementation();
    AParty* pPartyTwo = pArenaManager->GetPartyTwo_Implementation();
    bool isPartyOneDead = pPartyOne->IsPartyDead();
    bool isPartyTwoDead = pPartyTwo->IsPartyDead();
    ATurnsManager* pTurnsManager = UArenaUtilities::GetTurnsManager(this);
    int32 turnsLimit = pTurnsManager->GetTurnsLimit();
    bool isTurnsLimitReached = turnsLimit > 0 && turnsLimit <= pTurnsManager->GetCurrentTurn();
    if (isPartyOneDead || isPartyTwoDead || isTurnsLimitReached)
    {
        FArenaResults arenaResults;
        arenaResults.mIsPartyOneDead = isPartyOneDead;
        arenaResults.mIsPartyTwoDead = isPartyTwoDead;
        arenaResults.mpWinnerParty = nullptr;
        if (isPartyOneDead && isPartyTwoDead == false)
        {
            arenaResults.mpWinnerParty = pPartyTwo;
            check(IsValid(arenaResults.mpWinnerParty));
        }
        else if (isPartyTwoDead && isPartyOneDead == false)
        {
            arenaResults.mpWinnerParty = pPartyOne;
            check(IsValid(arenaResults.mpWinnerParty));
        }

        auto pGameInstance = Cast<UGameInstance_Unknown>(GetGameInstance());
        pGameInstance->mArenaResults = arenaResults;

        mIsWaitingForPlayersEoADecision = true;
        mpArenaGameState->EndOfArena(arenaResults);
        return true;
    }
    return false;
}

void AGameMode_ArenaFight::DetermineArenaGameMode()
{
    mArenaGameMode = EArenaGameMode::INVALID;
    auto pGameInstance = GetGameInstance<UGameInstance_Unknown>();
    if (IsValid(pGameInstance))
    {
        mArenaGameMode = pGameInstance->mArenaSetupInfo.mArenaGameMode;
    }
    if (mArenaGameMode == EArenaGameMode::INVALID)
    {
        auto pWorldSettings = Cast<AWorldSettings_Unknown>(GetWorldSettings());
        if (IsValid(pWorldSettings))
        {
            mArenaGameMode = pWorldSettings->mDefaultArenaGameMode;
        }
    }
}

void AGameMode_ArenaFight::TryStartNewTurn()
{
    if (IsValid(mpPlayerOne) && mpPlayerOne->GetArenaPlayerState()->mIsExecutionFinished &&
        (IsValid(mpPlayerTwo) == false || mpPlayerTwo->GetArenaPlayerState()->mIsExecutionFinished))
    {
        UE_LOG(ArenaCommonLog, Verbose, TEXT("All instances finished execution. Start planning new turn."));

        mpPlayerOne->GetArenaPlayerState()->mIsExecutionFinished = false;
        if (IsValid(mpPlayerTwo))
        {
            mpPlayerTwo->GetArenaPlayerState()->mIsExecutionFinished = false;
        }
        mIsServerExecutionFinished = false;
        mIsWaitingForPlayersFinishExecution = false;
        mIsWaitingForPlayersFinishPlanning = true;
        UArenaUtilities::GetTurnsManager(this)->StartPlanning();
    }
}

bool AGameMode_ArenaFight::IsWaitingForPlayersFinishPlanning() const
{
    //  May be changed to check of CurrentArenaPhase (PlanningPhase, ExecutionPahse etc.):
    return mIsWaitingForPlayersFinishPlanning;
}

bool AGameMode_ArenaFight::IsWaitingForPlayersFinishExecution() const
{
    //  May be changed to check of CurrentArenaPhase (PlanningPhase, ExecutionPahse etc.):
    return mIsWaitingForPlayersFinishExecution;
}

bool AGameMode_ArenaFight::IsWaitingForPlayersEoADecision() const
{
    return mIsWaitingForPlayersEoADecision;
}
