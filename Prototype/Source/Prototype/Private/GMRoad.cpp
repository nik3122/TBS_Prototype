// Fill out your copyright notice in the Description page of Project Settings.

#include "GMRoad.h"
#include "GMCamp.h"


AGMRoad::AGMRoad()
	: mIsActive(false)
    , mIsAvailable(true)
    , mpRoadSpline(nullptr)
	, mpStartCamp(nullptr)
	, mpEndCamp(nullptr)
{
	PrimaryActorTick.bCanEverTick = false;

	mpRoadSpline = CreateDefaultSubobject<USplineComponent>(TEXT("RoadSpline"));
	RootComponent = mpRoadSpline;
}

void AGMRoad::OnConstruction(const FTransform& transform)
{
    if (mpStartCamp != nullptr)
    {
        mpRoadSpline->SetLocationAtSplinePoint(0,
                                               mpStartCamp->GetActorLocation(),
                                               ESplineCoordinateSpace::World);
    }

    if (mpEndCamp != nullptr)
    {
        mpRoadSpline->SetLocationAtSplinePoint(mpRoadSpline->GetNumberOfSplinePoints() - 1,
                                               mpEndCamp->GetActorLocation(),
                                               ESplineCoordinateSpace::World);
    }

    Super::OnConstruction(transform);
}

void AGMRoad::BeginPlay()
{
    Super::BeginPlay();

    check(mpStartCamp != nullptr && mpEndCamp != nullptr && "Road isn't bound to two camps.");

    mpStartCamp->AddRoad(this);
    mpEndCamp->AddRoad(this);

    UpdateIsActive();
}

void AGMRoad::NotifyActorOnClicked(FKey buttonPressed)
{
    if (IsAvailable() && IsAvailable())
    {
        mEventRoadSelected.Broadcast(this);
    }
}

FVector AGMRoad::GetClosestEndingPosition(const AGMCamp* pCamp) const
{
	check((pCamp == mpStartCamp || pCamp == mpEndCamp) && "Trying to get clossest ending position of an UNBOUND camp.");
	float distanceRatio = (pCamp == mpStartCamp) ? 0.1f : 0.9f;
	return mpRoadSpline->GetLocationAtDistanceAlongSpline(distanceRatio * mpRoadSpline->GetSplineLength(), ESplineCoordinateSpace::World);
}

bool AGMRoad::IsActive() const
{
    return mIsActive;
}

void AGMRoad::UpdateIsActive()
{
	mIsActive = mpStartCamp->IsActive() || mpEndCamp->IsActive();
	OnSetActive();
}

FTransform AGMRoad::GetTransformAtCompleteness(float completeness, AGMCamp* pTargetCamp) const
{
	check((pTargetCamp == mpEndCamp || pTargetCamp == mpStartCamp) && "Camp is UNBOUND.");
	bool isReversed = pTargetCamp == mpStartCamp;
	completeness = FMath::Clamp(completeness, 0.0f, 1.0f);
	float distance = (isReversed ? 1.0f - completeness : completeness) * mpRoadSpline->GetSplineLength();
	FTransform transform = mpRoadSpline->GetTransformAtDistanceAlongSpline(distance, ESplineCoordinateSpace::World);
	FQuat normRotation = (isReversed ? FQuat::MakeFromEuler(FVector(0.0f, 0.0f, 180.0f)) : FQuat::Identity) * transform.GetRotation();
	transform.SetRotation(normRotation);
	return transform;
}

AGMCamp* AGMRoad::GetOppositeCamp(AGMCamp* pCamp) const
{
	check((pCamp == mpEndCamp || pCamp == mpStartCamp) && "Camp is UNBOUND.");
	return mpStartCamp == pCamp ? mpEndCamp : mpStartCamp;
}

float AGMRoad::GetRoadLength() const
{
	return mpRoadSpline->GetSplineLength();
}

bool AGMRoad::IsAvailable() const
{
    return mIsAvailable;
}

void AGMRoad::SetAvailable(bool isAvailable)
{
    mIsAvailable = isAvailable;
    OnSetAvailable();
}

void AGMRoad::GetStartCamp(AGMCamp*& pStartCamp, AGMCamp*& pEndCamp) const
{
    pStartCamp = mpStartCamp;
    pEndCamp = mpEndCamp;
}

void AGMRoad::SaveState(FGMRoadState& state)
{
    state.mIsAvailable = mIsAvailable;
    state.mRoadID = GetFName();
}

void AGMRoad::LoadState(const FGMRoadState& state)
{
    mIsAvailable = state.mIsAvailable;
}
