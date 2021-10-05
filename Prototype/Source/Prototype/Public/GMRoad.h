// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SplineComponent.h"
#include "CampaignSaveGame.h"
#include "GMRoad.generated.h"


class AGMCamp;


UCLASS()
class PROTOTYPE_API AGMRoad
    : public AActor
{
	GENERATED_BODY()

public:

    DECLARE_EVENT_OneParam(AGMRoad, FEventRoadSelected, AGMRoad* /*pSelectedRoad*/);
    //DECLARE_DELEGATE_OneParam(FEventRoadSelected, AGMRoad* /*pSelectedRoad*/);
    FEventRoadSelected mEventRoadSelected;

public:

	AGMRoad();

    virtual void OnConstruction(const FTransform& transform) override;

    virtual void BeginPlay() override;

    virtual void NotifyActorOnClicked(FKey buttonPressed) override;

	FVector GetClosestEndingPosition(const AGMCamp* pCamp) const;

    bool IsActive() const;

	void UpdateIsActive();

    void GetStartCamp(AGMCamp*& pStartCamp, AGMCamp*& pEndCamp) const;

    void SaveState(FGMRoadState& state);

    void LoadState(const FGMRoadState& state);

	UFUNCTION(BlueprintPure, Category = "Unknown|GlobalMap")
	FTransform GetTransformAtCompleteness(float completeness,
										  AGMCamp* pTargetCamp) const;

	UFUNCTION(BlueprintPure, Category = "Unknown|GlobalMap")
	AGMCamp* GetOppositeCamp(AGMCamp* pCamp) const;

	UFUNCTION(BlueprintPure, Category = "Unknown|GlobalMap")
	float GetRoadLength() const;

    UFUNCTION(BlueprintPure, Category = "Unknown|GlobalMap")
    bool IsAvailable() const;

    UFUNCTION(BlueprintCallable, Category = "Unknown|GlobalMap")
    void SetAvailable(bool isAvailable);

protected:

	UPROPERTY(BlueprintReadOnly, Category = "Unknown|GlobalMap")
	bool mIsActive;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Unknown|GlobalMap")
    bool mIsAvailable;

	UPROPERTY(BlueprintReadWrite, Category = "Unknown|GlobalMap")
	USplineComponent* mpRoadSpline;

	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Unknown|GlobalMap")
	AGMCamp* mpStartCamp;

	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Unknown|GlobalMap")
	AGMCamp* mpEndCamp;

protected:

	UFUNCTION(BlueprintImplementableEvent, Category = "Unknown|GlobalMap")
	void OnSetActive();

    UFUNCTION(BlueprintImplementableEvent, Category = "Unknown|GlobalMap")
    void OnSetAvailable();

};
