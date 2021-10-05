

#include "Arena/ArenaManager.h"
#include "Arena/ActionMap.h"
#include "Arena/ArenaUtilities.h"
#include "Arena/FireSystem.h"
#include "Arena/InfectionSystem.h"
#include "Arena/TurnsManager.h"
#include "Arena/PassMap.h"
#include "Arena/Party.h"
#include "Arena/Weather.h"
#include "GameInstance_Unknown.h"
#include "Net/UnrealNetwork.h"
#include "UnknownCommon.h"


AArenaManager::AArenaManager(const FObjectInitializer& objectInitializer)
    : Super(objectInitializer)
{
    bReplicates = true;
    bAlwaysRelevant = true;
}

void AArenaManager::RegisterSystem(AActor* pSystem)
{
    //  ~!~ As there is no GameplaySystem_Interface - nothing to check.
    TrySetupSystems();
}

void AArenaManager::SetParties(AParty* pPartyOne, AParty* pPartyTwo)
{
    mpPartyOne = pPartyOne;
    mpPartyTwo = pPartyTwo;
}

bool AArenaManager::IsArenaSetup() const
{
    return mIsArenaSetup;
}

AParty* AArenaManager::GetPartyOne_Implementation()
{
    checkSlow(IsValid(mpPartyOne) && "Parties not spawned yet.");
    return mpPartyOne;
}

AParty* AArenaManager::GetPartyTwo_Implementation()
{
    checkSlow(IsValid(mpPartyTwo) && "Parties not spawned yet.");
    return mpPartyTwo;
}

AParty* AArenaManager::GetPlayerParty_Implementation()
{
    checkSlow(IsValid(mpPartyOne) && "Parties not spawned yet.");
    return mpPartyOne;
}

AParty* AArenaManager::GetAIParty_Implementation()
{
    checkSlow(IsValid(mpPartyTwo) && "Parties not spawned yet.");
    return mpPartyTwo;
}

ATurnsManager* AArenaManager::GetTurnsManager_Implementation()
{
    if (mAreSystemsSetup == false)
    {
        UE_LOG(ArenaCommonLog, Warning, TEXT("Systems are not setup yet! Subscribe for AArenaManager::mEventSystemsSetup to postpone an interaction with the systems."));
    }
    return mpTurnsManager;
}

APassMap* AArenaManager::GetPassMap_Implementation()
{
    if (mAreSystemsSetup == false)
    {
        UE_LOG(ArenaCommonLog, Warning, TEXT("Systems are not setup yet! Subscribe for AArenaManager::mEventSystemsSetup to postpone an interaction with the systems."));
    }
    return mpPassMap;
}

AActionMap* AArenaManager::GetActionMap_Implementation()
{
    if (mAreSystemsSetup == false)
    {
        UE_LOG(ArenaCommonLog, Warning, TEXT("Systems are not setup yet! Subscribe for AArenaManager::mEventSystemsSetup to postpone an interaction with the systems."));
    }
    return mpActionMap;
}

AFireSystem* AArenaManager::GetFireSystem_Implementation()
{
    if (mAreSystemsSetup == false)
    {
        UE_LOG(ArenaCommonLog, Warning, TEXT("Systems are not setup yet! Subscribe for AArenaManager::mEventSystemsSetup to postpone an interaction with the systems."));
    }
    return mpFireSystem;
}

AInfectionSystem* AArenaManager::GetInfectionSystem_Implementation()
{
    if (mAreSystemsSetup == false)
    {
        UE_LOG(ArenaCommonLog, Warning, TEXT("Systems are not setup yet! Subscribe for AArenaManager::mEventSystemsSetup to postpone an interaction with the systems."));
    }
    return mpInfectionSystem;
}

AWeather* AArenaManager::GetWeather_Implementation()
{
    if (mAreSystemsSetup == false)
    {
        UE_LOG(ArenaCommonLog, Warning, TEXT("Systems are not setup yet! Subscribe for AArenaManager::mEventSystemsSetup to postpone an interaction with the systems."));
    }
    return mpWeather;
}

bool AArenaManager::AreSystemsSetup_Implementation() const
{
    return mAreSystemsSetup;
}

void AArenaManager::OnRep_PartyReplicated()
{
    if (IsValid(mpPartyOne) &&
        IsValid(mpPartyTwo))
    {
        UE_LOG(ArenaCommonLog, Log, TEXT("AArenaManager: Party One and Party Two are relicated."));
        // Both parties replicated. Can notify server that client is ready.
        // Can notify player controller.
    }
}

void AArenaManager::OnActionsSubmittingFinished()
{
    UE_LOG(ArenaCommonLog, Verbose, TEXT("Action submitting finished."));

    mpActionMap->ResolveActionConflicts();
    mpFireSystem->UpdateFireDistribution();
    mpFireSystem->ProcessAllMovements();
    mpTurnsManager->ExecuteTurn();
}

void AArenaManager::BeginPlay()
{
    Super::BeginPlay();

    check(mpTurnsManager != nullptr && "TurnsManager must be set in ArenaManager.");
    check(mpPassMap != nullptr && "PassMap must be set in ArenaManager.");
    check(mpActionMap != nullptr && "ActionMap must be set in ArenaManager.");
    check(mpFireSystem != nullptr && "FireSystem must be set in ArenaManager.");
    check(mpWeather != nullptr && "Weather must be set in ArenaManager.");

    TrySetupSystems();
}

void AArenaManager::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(AArenaManager, mpPartyOne);
    DOREPLIFETIME(AArenaManager, mpPartyTwo);
}

bool AArenaManager::CheckSystemReadinessForSetup(AActor* pSystem) const
{
    return IsValid(pSystem) && pSystem->HasActorBegunPlay();
}

void AArenaManager::TrySetupSystems()
{
    if (mAreSystemsSetup == false &&
        CheckSystemReadinessForSetup(mpTurnsManager) &&
        CheckSystemReadinessForSetup(mpPassMap) &&
        CheckSystemReadinessForSetup(mpFireSystem) &&
        CheckSystemReadinessForSetup(mpActionMap) &&
        CheckSystemReadinessForSetup(mpWeather))
    {
        mpTurnsManager->SetupSystem(mpActionMap);
        mpPassMap->SetupSystem(mpTurnsManager);
        mpFireSystem->SetupSystem(mpTurnsManager, mpPassMap);
        mpInfectionSystem->SetupSystem(mpPassMap);
        mpActionMap->SetupSystem(mpTurnsManager);
        mpWeather->SetupSystem();
        mAreSystemsSetup = true;

        mpActionMap->mEventActionsSubmittingFinished.BindUObject(this, &AArenaManager::OnActionsSubmittingFinished);

        mEventSystemsSetup.Broadcast();

        //  ~!~HACK~TODO~ EventArenaSetup must be dispatched when all actors (arena units, traps, etc.) are synced with the server:
        auto delayedArenaSetup = [this]() {
            mIsArenaSetup = true;
            mEventArenaSetup.ExecuteIfBound();
        };
        FTimerHandle tempTimer;
        float duration = 2.f;
        GetWorld()->GetTimerManager().SetTimer(tempTimer,
                                               delayedArenaSetup,
                                               duration,
                                               false);
    }
}
