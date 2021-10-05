// Fill out your copyright notice in the Description page of Project Settings.

#include "GMPawn.h"
#include "GMRoad.h"
#include "GMCamp.h"
#include "UnknownCommon.h"
#include "GameInstance_Unknown.h"


AGMPawn::AGMPawn()
    : mpTravelTimelineCurve(nullptr)
    , mPartyMembersCountLimit(0)
    , mTravelSpeed(100.0f)
    , mpCurrentCamp(nullptr)
    , mpRoadToTravel(nullptr)
    , mpTargetCamp(nullptr)
    , mLeftTime(0.0f)
    , mTimeToTravel(1.0f)
    , mIsTraveling(false)
{
	PrimaryActorTick.bCanEverTick = true;

    SetCanBeDamaged(false);
    bCollideWhenPlacing = false;
    AutoPossessAI = EAutoPossessAI::Disabled;
    SpawnCollisionHandlingMethod = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
}

void AGMPawn::BeginPlay()
{
	Super::BeginPlay();
	

}

void AGMPawn::Tick(float deltaSeconds)
{
    Super::Tick(deltaSeconds);
    
    if (mIsTraveling)
    {
        check(mTickTravel != nullptr && "Tick function should be set.");
        (this->*mTickTravel)(deltaSeconds);
    }
}

void AGMPawn::StartTravel(AGMCamp* pTargetCamp, ECampTravel campTravelType)
{
    check(mIsTraveling == false && "Trying to travel while traveling.");
    check(pTargetCamp != nullptr && "Trying to travel to an invalid camp.");
    check(campTravelType != ECampTravel::CampTravel_None && "Camp travel type should be valid.");
    mpTargetCamp = pTargetCamp;
    switch (campTravelType)
    {
    case ECampTravel::CampTravel_Neighbour:
        {
            mpRoadToTravel = nullptr;
            for (AGMRoad* pRoad : mpCurrentCamp->GetRoads())
            {
                if (pRoad->GetOppositeCamp(mpCurrentCamp) == pTargetCamp)
                {
                    mpRoadToTravel = pRoad;
                    break;
                }
            }
            check(mpRoadToTravel != nullptr && "CampTravel_Neighbour: target camp is not neighbour!");
            mTimeToTravel = mpRoadToTravel->GetRoadLength() / mTravelSpeed;
            mpTargetCamp = pTargetCamp;
            mLeftTime = mTimeToTravel;
            mTickTravel = &AGMPawn::TickTravel_Neighbour;
        }
        break;

    case ECampTravel::CampTravel_Direct:
    case ECampTravel::CampTravel_PathFind:
        {
            mOldCampPosition = GetActorLocation();
            mLeftTime = mTimeToTravel;
            mTickTravel = &AGMPawn::TickTravel_Direct;
        }
        break;

    case ECampTravel::CampTravel_Unconsciously:
        {
            mTickTravel = &AGMPawn::TickTravel_Unconsciously;
        }
        break;
    }
    LeaveCurrentCamp();
    mIsTraveling = true;
}

void AGMPawn::StopTravel()
{
    mIsTraveling = false;
    EnterTargetCamp();
}

bool AGMPawn::IsTraveling() const
{
    return mIsTraveling;
}

void AGMPawn::SetCurrentCamp(AGMCamp* pCamp)
{
    check(mIsTraveling == false && "Trying to set current camp while traveling.");
    AGMCamp* pOldCamp = mpCurrentCamp;
    mpCurrentCamp = pCamp;
    if (mpCurrentCamp != nullptr)
    {
        SetActorLocation(mpCurrentCamp->GetActorLocation());
    }
    mEventCampChanged.ExecuteIfBound(pOldCamp, mpCurrentCamp);
}

AGMCamp* AGMPawn::GetCurrentCamp() const
{
    return mpCurrentCamp;
}

void AGMPawn::SaveState(FGMPawnState& state)
{
    //  Nothing to do here...
}

void AGMPawn::LoadState(const FGMPawnState& state)
{
    //  Nothing to do here...
}

void AGMPawn::AddPartyMember(const FName& creatureName)
{
    if (mPartyMembers.Num() >= mPartyMembersCountLimit)
    {
#if !UE_BUILD_SHIPPING
        UE_LOG(GlobalMapCommonLog, Error, TEXT("ERROR: Party members limit is reached! Can't add new party member."));
#endif // #if !UE_BUILD_SHIPPING
        return;
    }

#if !UE_BUILD_SHIPPING
    UGameInstance_Unknown* pGameInstance = UUnknownCommon::GetGameInstanceUnknown(this);
    FCreatureInfo* pCreatureInfo = pGameInstance->mpCreaturesInfoTable->FindRow<FCreatureInfo>(creatureName, "Can't find creature.");
    if (pCreatureInfo == nullptr)
    {
        UE_LOG(GlobalMapCommonLog, Error, TEXT("ERROR: Added to GMPawn party creature %s doesn't exist."), *creatureName.ToString());
        return;
    }
#endif // #if !UE_BUILD_SHIPPING

    mPartyMembers.Add(creatureName);
    mEventMembersChanged.ExecuteIfBound();
}

void AGMPawn::RemovePartyMember(const FName& creatureName)
{
    int32 removedCount = mPartyMembers.RemoveSingle(creatureName);
    if (removedCount == 0)
    {
#if !UE_BUILD_SHIPPING
        UE_LOG(GlobalMapCommonLog, Error, TEXT("ERROR: The party member \"%s\" isn't present in the party. Thus it can't be removed."), *creatureName.ToString());
#endif // #if !UE_BUILD_SHIPPING
    }
    mEventMembersChanged.ExecuteIfBound();
}

const TArray<FName>& AGMPawn::GetPartyMembers() const
{
    return mPartyMembers;
}

void AGMPawn::LeaveCurrentCamp()
{
    mEventTravelStarted.ExecuteIfBound();
    SetCurrentCamp(nullptr);
}

void AGMPawn::EnterTargetCamp()
{
    SetCurrentCamp(mpTargetCamp);
    mpTargetCamp = nullptr;
    //  ~!~ We assume that at the moment this event is fired the pawn is already at the target camp:
    mEventTravelFinished.ExecuteIfBound();
}

void AGMPawn::TickTravel_Neighbour(float deltaSeconds)
{
    float completeness = mpTravelTimelineCurve->GetFloatValue((mTimeToTravel - mLeftTime) / mTimeToTravel);
    FTransform transform = mpRoadToTravel->GetTransformAtCompleteness(completeness, mpTargetCamp);
    FVector newPos = transform.GetLocation();
    SetActorLocation(newPos);
    if (mLeftTime <= deltaSeconds)
    {
        StopTravel();
    }
    mLeftTime = FMath::Max(mLeftTime - deltaSeconds, 0.0f);
}

void AGMPawn::TickTravel_Direct(float deltaSeconds)
{
    FVector newCampPosition = mpTargetCamp->GetActorLocation();
    FVector newPosition = FMath::Lerp(
        mOldCampPosition,
        newCampPosition,
        FMath::Max((mTimeToTravel - mLeftTime) / mTimeToTravel, 0.0f)
    );
    //FVector travelDirection = newCampPosition - mOldCampPosition;
    //FTransform transform(travelDirection.Rotation(), newPosition);
    //FVector newPos = transform.GetLocation();
    SetActorLocation(newPosition);
    if (mLeftTime <= deltaSeconds)
    {
        StopTravel();
    }
    mLeftTime = FMath::Max(mLeftTime - deltaSeconds, 0.0f);
}

void AGMPawn::TickTravel_Unconsciously(float deltaSeconds)
{
    SetActorLocation(mpTargetCamp->GetActorLocation());
    StopTravel();
}
