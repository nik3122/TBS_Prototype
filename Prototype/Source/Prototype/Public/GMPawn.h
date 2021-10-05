// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "CampaignSaveGame.h"
#include "DialogueSystem.h"
#include "GMPawn.generated.h"


class AGMCamp;
class AGMRoad;


UCLASS()
class PROTOTYPE_API AGMPawn : public APawn
{
	GENERATED_BODY()

public:

    DECLARE_DELEGATE(FEventTravelStarted);
    FEventTravelStarted mEventTravelStarted;

    DECLARE_DELEGATE(FEventTravelFinished);
    FEventTravelFinished mEventTravelFinished;

    DECLARE_DELEGATE_TwoParams(FEventCampChanged, AGMCamp* /*pOldCamp*/, AGMCamp* /*pNewCamp*/);
    FEventCampChanged mEventCampChanged;

    DECLARE_DELEGATE(FEventMembersChanged);
    FEventMembersChanged mEventMembersChanged;

public:

    AGMPawn();

    void BeginPlay() override;

    void Tick(float deltaSeconds) override;

    void StartTravel(AGMCamp* pTargetCamp, ECampTravel campTravelType);

    void StopTravel();

    bool IsTraveling() const;

    void SetCurrentCamp(AGMCamp* pCamp);

    AGMCamp* GetCurrentCamp() const;

    void SaveState(FGMPawnState& state);

    void LoadState(const FGMPawnState& state);

    UFUNCTION(BlueprintCallable, Category = "Unknown|GlobalMap")
    void AddPartyMember(const FName& creatureName);

    UFUNCTION(BlueprintCallable, Category = "Unknown|GlobalMap")
    void RemovePartyMember(const FName& creatureName);

    UFUNCTION(BlueprintPure, Category = "Unknown|GlobalMap")
    const TArray<FName>& GetPartyMembers() const;

protected:

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Unknown|GlobalMap")
    UCurveFloat* mpTravelTimelineCurve;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Unknown|GlobalMap")
    int32 mPartyMembersCountLimit;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Unknown|GlobalMap")
    float mTravelSpeed;

protected:

    void (AGMPawn::*mTickTravel)(float deltaSeconds);

private:

    AGMCamp* mpCurrentCamp;

    AGMRoad* mpRoadToTravel;

    AGMCamp* mpTargetCamp;

    //  ~T~
    float mLeftTime;
    float mTimeToTravel;

    FVector mOldCampPosition;

    bool mIsTraveling;

    TArray<FName> mPartyMembers;

private:

    void LeaveCurrentCamp();

    void EnterTargetCamp();

    void TickTravel_Neighbour(float deltaSeconds);

    void TickTravel_Direct(float deltaSeconds);

    void TickTravel_Unconsciously(float deltaSeconds);
};
