// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CampaignSaveGame.h"
#include "DialogueSystem.h"
#include "GMCamp.generated.h"


class AGMRoad;


UCLASS()
class PROTOTYPE_API AGMCamp : public AActor
{
	GENERATED_BODY()

public:

	DECLARE_DELEGATE_TwoParams(FEventRoadSelected, AGMRoad* /*pRoad*/, AGMCamp* /*pTargetCamp*/);
	FEventRoadSelected mEventRoadSelected;

public:

	AGMCamp();

    const TArray<AGMRoad*>& GetRoads() const;

    bool IsActive() const;

    bool TryGetActionBlock(const FName& actionBlockID, FActionBlock& actionBlock);

    void SaveState(FGMCampState& state);

    void LoadState(const FGMCampState& state);

	UFUNCTION(BlueprintCallable, Category = "Unknown|GlobalMap")
	void SetActive(bool isActive);

	UFUNCTION(BlueprintCallable, Category = "Unknown|GlobalMap")
	void AddRoad(AGMRoad* pRoad);

    UFUNCTION(BlueprintPure, Category = "Unknown|GlobalMap")
    FName GetCampID() const;

    UFUNCTION(BlueprintPure, Category = "Unknown|GlobalMap")
    const FCampData& GetCampData() const;

    UFUNCTION(BlueprintPure, Category = "Unknown|GlobalMap")
    const FText& GetCampName() const;

protected:

    UPROPERTY(BlueprintReadOnly, Category = "Unknown")
    TArray<AGMRoad*> mRoads;

    UPROPERTY(BlueprintReadOnly, Category = "Unknown")
    bool mIsActive;

    bool mIsInitialized;

    FCampData mCampData;

    UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Unknown")
    FText mCampName;

protected:

	UFUNCTION(BlueprintImplementableEvent, Category = "Unknown|GlobalMap")
	void OnSetActive();	

private:

    void OnRoadSelected(AGMRoad* pRoad);

    void InitCampData();
};
