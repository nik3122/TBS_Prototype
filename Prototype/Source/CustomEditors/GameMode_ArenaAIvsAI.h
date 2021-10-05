#pragma once


#include "CoreMinimal.h"
#include "Arena/Controller_ArenaAI.h"
#include "Arena/GameMode_ArenaFight.h"
#include "GameMode_ArenaAIvsAI.generated.h"


UCLASS(Blueprintable)
class AGameMode_ArenaFightAIvsAI
    : public AGameMode_ArenaFight
{
    GENERATED_BODY()

public:

    AGameMode_ArenaFightAIvsAI();

protected:

    AController_ArenaAI* mpControllerPartyPlayer;

protected:

    void InitGame(const FString& mapName,
                  const FString& options,
                  FString& errorMessage) override;

    void InitGameState() override;

    void BeginPlay() override;

    //void SetupParties() final;

    void AIMakeDecision() final;

    UFUNCTION()
    void OnUnlockPlayerControl();

    void DelayStartTurn();
};