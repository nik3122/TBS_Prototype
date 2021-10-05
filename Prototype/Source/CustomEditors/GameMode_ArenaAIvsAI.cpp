

#include "GameMode_ArenaAIvsAI.h"
#include "CustomEditors.h"
#include "Arena/ArenaUtilities.h"
#include "Arena/Controller_ArenaAI.h"
#include "Arena/GameState_Arena.h"
#include "Arena/Party.h"
#include "Arena/TurnsManager.h"
#include "UnknownCommon.h"
#include "TimerManager.h"
#include "GameInstance_Unknown.h"


AGameMode_ArenaFightAIvsAI::AGameMode_ArenaFightAIvsAI()
{
    DefaultPawnClass = ASpectatorPawn::StaticClass();
    HUDClass = nullptr;
    PlayerControllerClass = APlayerController::StaticClass();
}

void AGameMode_ArenaFightAIvsAI::InitGame(const FString& mapName,
                                          const FString& options,
                                          FString& errorMessage)
{
    Super::InitGame(mapName,
                    options,
                    errorMessage);

    //mpTurnsManager->mEventUnlockPlayerControl.AddDynamic(this, &AGameMode_ArenaFightAIvsAI::OnUnlockPlayerControl);
}

void AGameMode_ArenaFightAIvsAI::InitGameState()
{
    Super::InitGameState();

    //mpArenaGameState->mEventSystemsSetup.AddDynamic(this, &AGameMode_ArenaFightAIvsAI::OnUnlockPlayerControl);
}

void AGameMode_ArenaFightAIvsAI::BeginPlay()
{
    Super::BeginPlay();

    //UGameplayStatics::GetPlayerController(this, 0)->SetActorLocation(FVector(10000, 10000, 10000));
    //TActorIterator<APlayerStart> 
    //UGameplayStatics::GetPlayerController(this, 0)->ClientSetLocation(FVector(670, 980, 720), FRotator::ZeroRotator);

    OnUnlockPlayerControl();
}

/*
void AGameMode_ArenaFightAIvsAI::SetupParties()
{
    auto& arenaSetupInfo = UUnknownCommon::GetGameInstanceUnknown(this)->mArenaSetupInfo;

    AParty* pPartyOne = mpArenaGameState->GetPartyOne();
    pPartyOne->SetPartyInfo({ FText::FromString(TEXT("One")), FLinearColor::Blue });
    AParty* pPartyTwo = mpArenaGameState->GetPartyOne();
    pPartyTwo->SetPartyInfo({ FText::FromString(TEXT("Two")), FLinearColor::Red });

    //  Possess parties according to arena setup info:
    mpPlayerOne->Possess(pPartyOne);
    if (arenaSetupInfo.mIsOpponentAI)
    {
        FActorSpawnParameters spawnParams;
        spawnParams.Owner = this;
        spawnParams.bNoFail = true;
        mpControllerPartyAI = GetWorld()->SpawnActor<AController_ArenaAI>(spawnParams);
        mpControllerPartyAI->Possess(pPartyTwo);
        mpControllerPartyAI->SetHeuristicsWeights(UUnknownCommon::GetGameInstanceUnknown(this)->mAIHeuristics);
    }
    else if (arenaSetupInfo.mIsHotseat == false)
    {
        check(mpPlayerTwo != nullptr);
        mpPlayerTwo->Possess(pPartyTwo);
    }

    APassMap* pPassMap = mpArenaGameState->GetPassMap();

    TArray<FIntPoint> spawnTilesPlayerParty;
    TArray<FIntPoint> spawnTilesAIParty;
    pPassMap->GetSpawnTiles(spawnTilesPlayerParty, spawnTilesAIParty);

    UGameInstance_Unknown* pGameInstance = Cast<UGameInstance_Unknown>(GetGameInstance());
    check(pGameInstance != nullptr && "Game instance is not of GameInstance_Unknown type!");
    TArray<TSubclassOf<ACreatureBase>>& playerPartyMembers = arenaSetupInfo.mPartyOneMembers;
    TArray<TSubclassOf<ACreatureBase>>& AIPartyMembers = arenaSetupInfo.mPartyOneMembers;

    checkf(spawnTilesPlayerParty.Num() >= playerPartyMembers.Num(), TEXT("Not enough spawn tiles (%u) for player party (%u)!"), spawnTilesPlayerParty.Num(), playerPartyMembers.Num());
    for (int32 i = 0; i < playerPartyMembers.Num(); ++i)
    {
        FVector spawnPosition = pPassMap->GetTilePositionWorld(spawnTilesPlayerParty[i]);
        FRotator spawnRotation = FRotator::ZeroRotator;
        ACreatureBase* pSpawnedCreature = GetWorld()->SpawnActor<ACreatureBase>(playerPartyMembers[i], spawnPosition, spawnRotation);
        pPartyOne->AddMember(pSpawnedCreature);
    }

    checkf(spawnTilesAIParty.Num() >= AIPartyMembers.Num(), TEXT("Not enough spawn tiles (%u) for AI party (%u)!"), spawnTilesPlayerParty.Num(), playerPartyMembers.Num());
    for (int32 i = 0; i < AIPartyMembers.Num(); ++i)
    {
        FVector spawnPosition = pPassMap->GetTilePositionWorld(spawnTilesAIParty[i]);
        FRotator spawnRotation = FRotator::ZeroRotator;
        ACreatureBase* pSpawnedCreature = GetWorld()->SpawnActor<ACreatureBase>(AIPartyMembers[i], spawnPosition, spawnRotation);
        pPartyTwo->AddMember(pSpawnedCreature);
    }
}
*/

void AGameMode_ArenaFightAIvsAI::AIMakeDecision()
{
    mpControllerPartyPlayer->AIMakeDecision();
    mpControllerPartyAI->AIMakeDecision();
}

void AGameMode_ArenaFightAIvsAI::OnUnlockPlayerControl()
{
    FTimerHandle tempTimerHandle;
    GetWorldTimerManager().SetTimer(tempTimerHandle,
                                    this,
                                    &AGameMode_ArenaFightAIvsAI::DelayStartTurn,
                                    1.0f,
                                    false);
}

void AGameMode_ArenaFightAIvsAI::DelayStartTurn()
{
    //mpTurnsManager->ExecuteTurn();
}
