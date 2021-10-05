// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "TiledMovement.h"
#include "PassMap.h"
#include "Damageable.h"
#include "StatusEffectsManager.h"
#include "Trap_Base.generated.h"


UCLASS()
class PROTOTYPE_API ATrap_Base
    : public AActor
{
    GENERATED_BODY()

public:

    bool IsInterruptingMovement() const;

    UTexture2D* GetTrapImage() const;

    void SetupTrap(const FIntPoint& placementTile);

    void SetCreatureTrapped(ACreatureBase* pTrappedCreature, int32 executionPriority);

    void BeginPlay() override;

protected:

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Unknown|Arena")
    bool mIsInterruptingMovement = true;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Unknown|Arena")
    UTexture2D* mpTrapImage = nullptr;

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Unknown|Arena")
    FDamageableTask mDamageableTask;

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Unknown|Arena")
    TArray<FStatusEffectTask> mStatusEffectTasks;

    UPROPERTY(BlueprintReadOnly, Category = "Unknown|Arena")
    ACreatureBase* mpTrappedCreature = nullptr;

    UPROPERTY(BlueprintReadOnly, Category = "Unknown|Arena")
    int32 mExecutionPriority = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Unknown|Arena")
    FIntPoint mPlacementTile;

protected:

    UFUNCTION(BlueprintNativeEvent, Category = "Unknown|Arena")
    void OnSetupTrap();
    void OnSetupTrap_Implementation();

    /**
        By default delivers damage and activates status effects for mpTrappedCreature.
        Children may override it for additional animation playing and custom reaction. Dont forget to call parent!
    */
    UFUNCTION(BlueprintNativeEvent, Category = "Unknown|Arena")
    void ApplyTrap();
    void ApplyTrap_Implementation();

    UFUNCTION()
    void OnTrappedCreatureReachedTile(UTiledMovement* pTiledMovement, const FIntPoint& tile);

    /**
        By default simply destroys itself.
    */
    UFUNCTION(BlueprintNativeEvent, Category = "Unknown|Arena")
    void OnTriggered();
    void OnTriggered_Implementation();

};
