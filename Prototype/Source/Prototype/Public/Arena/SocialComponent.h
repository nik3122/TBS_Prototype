// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Arena/Party.h"
#include "Components/ActorComponent.h"
#include "SocialComponent.generated.h"


class AParty;
class ACreatureBase;


//  ~TODO~ Rename PartyMemberComponent.
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent), Category = "Unknown|Social")
class PROTOTYPE_API USocialComponent : public UActorComponent
{
    GENERATED_BODY()

public:

    USocialComponent();

    void SetParty(AParty* pParty);

    UFUNCTION(BlueprintPure, Category = "Unknown|Social")
    AParty* GetParty() const;

    UFUNCTION(BlueprintPure, Category = "Unknown|Social")
    AParty* GetEnemyParty() const;

    UFUNCTION(BlueprintPure, Category = "Unknown|Social")
    void GetAllies(TArray<ACreatureBase*>& allyCreatures) const;

    UFUNCTION(BlueprintPure, Category = "Unknown|Social")
    void GetEnemies(TArray<ACreatureBase*>& enemyCreatures) const;

protected:

    void BeginPlay() override;

    void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:

    UPROPERTY(Replicated, BlueprintReadOnly, Category = "Unknown|Social")
    AParty* mpParty;
};
