// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Controller.h"
#include "Party.h"
#include "Action_Base.h"
#include "ArenaAIConfig.h"
#include "Controller_ArenaAI.generated.h"


UCLASS()
class PROTOTYPE_API AController_ArenaAI
    : public AController
{
    GENERATED_BODY()
    
public:

    AController_ArenaAI();

    void SetAIConfig(const FArenaAIConfig& arenaAIConfig);

    void AIMakeDecision();

protected:

    UPROPERTY(BlueprintReadOnly, Category = "Arena")
    AParty* mpParty;

    FArenaAIConfig mArenaAIConfig;

protected:

    void BeginPlay() override;

    void OnPossess(APawn* pPossessedPawn) override;
};
