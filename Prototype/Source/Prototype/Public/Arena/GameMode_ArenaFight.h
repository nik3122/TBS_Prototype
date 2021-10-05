// Fill out your copyright notice in the Description page of Project Settings.

#pragma once


#include "Arena/ArenaManager_Interface.h"
#include "Arena/ArenaResults.h"
#include "Arena/ArenaSetupInfo.h"
#include "GameFramework/GameModeBase.h"
#include "Version.h"
#include "GameMode_ArenaFight.generated.h"


class AParty;
class APlayerController_ArenaFight;
class AController_ArenaAI;
class AGameState_Arena;
struct FWeatherState;

/**
    
 */
UCLASS(Blueprintable)
class PROTOTYPE_API AGameMode_ArenaFight
    : public AGameModeBase
{
    GENERATED_BODY()

public:

    AGameMode_ArenaFight();

    EArenaGameMode GetArenaGameMode() const;

    void OnPlayerArenaSetup(APlayerController* pPlayer);

    bool IsWaitingForPlayersFinishPlanning() const;

    void OnPlayerPlanningFinished(APlayerController* pPlayer);

    void OnPlayerExecutionFinished(APlayerController* pPlayer);

    void OnPlayerEoARetry(APlayerController* pPlayer);

    UFUNCTION(BlueprintCallable, Category = "Arena")
    void LeaveArena();

protected:

    UPROPERTY(BlueprintReadOnly, Category = "Arena")
    AGameState_Arena* mpArenaGameState;

    UPROPERTY(BlueprintReadOnly, Category = "Arena")
    EArenaGameMode mArenaGameMode = EArenaGameMode::INVALID;

    UPROPERTY(BlueprintReadOnly, Category = "Arena")
    APlayerController_ArenaFight* mpPlayerOne = nullptr;

    UPROPERTY(BlueprintReadOnly, Category = "Arena")
    APlayerController_ArenaFight* mpPlayerTwo = nullptr;

    UPROPERTY(BlueprintReadOnly, Category = "Arena")
    AController_ArenaAI* mpControllerPartyAI;

    //  If true - forces allways to skip action's execution. Usefull for auto-tests, AI balance, map balance and so on.
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Arena")
    bool mSkipExecution = false;

protected:

    void StartPlay() override;

    void InitGame(const FString& mapName,
                  const FString& options,
                  FString& errorMessage) override;

    void InitGameState() override;

    void BeginPlay() override;

    virtual void SpawnAndSetupParties();

    void PreLogin(const FString& options, const FString& address, const FUniqueNetIdRepl& uniqueId, FString& errorMessage) override;

    void GenericPlayerInitialization(AController* pController) override;

    bool ArePlayersReady() const;

    void TryBeginBattle();

    void Logout(AController* exiting) override;

    UFUNCTION()
    void OnSystemsSetup();

    UFUNCTION()
    void OnTurnFinished();

private:

    bool mIsArenaSetupPlayerOne = false;
    bool mIsArenaSetupPlayerTwo = false;
    bool mIsServerExecutionFinished = false;

    bool mIsWaitingForPlayersFinishPlanning = false;
    bool mIsWaitingForPlayersFinishExecution = false;
    bool mIsWaitingForPlayersEoADecision = false;

private:

    virtual void AIMakeDecision();

    void StartNewTurnOrFinishBattle();

    bool TryFinishBattle();

    void DetermineArenaGameMode();

    void TryStartNewTurn();

    bool IsWaitingForPlayersFinishExecution() const;

    bool IsWaitingForPlayersEoADecision() const;
};
