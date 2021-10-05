// Fill out your copyright notice in the Description page of Project Settings.

#include "GMCamp.h"
#include "GMRoad.h"
#include "GameMode_GlobalMap.h"
#include "UnknownCommon.h"


AGMCamp::AGMCamp()
    : mIsActive(false)
    , mIsInitialized(false)
{
	PrimaryActorTick.bCanEverTick = false;

    SetCanBeDamaged(false);
    bCollideWhenPlacing = false;
    SpawnCollisionHandlingMethod = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
}

const TArray<AGMRoad*>& AGMCamp::GetRoads() const
{
    return mRoads;
}

void AGMCamp::SetActive(bool isActive)
{
	mIsActive = isActive;
    if (mIsActive && mIsInitialized == false)
    {
        InitCampData();
    }
	for (auto pRoad : mRoads)
	{
		pRoad->UpdateIsActive();
	}
	OnSetActive();
}

bool AGMCamp::IsActive() const
{
	return mIsActive;
}

bool AGMCamp::TryGetActionBlock(const FName& actionBlockID, FActionBlock& actionBlock)
{
    //  ~PERF~ Action blocks can be placed into the map:
    for (auto& ab : GetCampData().ActionBlocks)
    {
        if (ab.ID == actionBlockID)
        {
            actionBlock = ab;
            return true;
        }
    }
    return false;
}

void AGMCamp::SaveState(FGMCampState& state)
{
    state.mCampID = GetFName();
}

void AGMCamp::LoadState(const FGMCampState& state)
{
    //  Nothing to do...
}

void AGMCamp::AddRoad(AGMRoad* pRoad)
{
    check(pRoad != nullptr && "Added road is invalid.");
	mRoads.Add(pRoad);
    pRoad->UpdateIsActive();
    //  ~PERF~ If we have to load a whole huge GM and switch between GM and Arenas frequently it would be better
    //  to add callbacks only when camp is activated and release them when deactivated.
    pRoad->mEventRoadSelected.AddUObject(this, &AGMCamp::OnRoadSelected);
}

FName AGMCamp::GetCampID() const
{
    return FName(*GetName());
}

void AGMCamp::OnRoadSelected(AGMRoad* pRoad)
{
    check(pRoad != nullptr && "RoadActor is invalid.");
    mEventRoadSelected.ExecuteIfBound(pRoad, pRoad->GetOppositeCamp(this));
}

void AGMCamp::InitCampData()
{
    // ~R~ Don't like the direct dependency on GameMode_GlobalMap. But why not...
    UDataTable* pCampDataTable = UUnknownCommon::GetGlobalMapGameMode(this)->GetCampDataTable();
    check(pCampDataTable != nullptr && "Camp Data table is invalid.");
    FCampData* pCampData = pCampDataTable->FindRow<FCampData>(GetCampID(), "Camp Data is not detected.");
    if (pCampData != nullptr)
    {
        mCampData = *pCampData;
        mIsInitialized = true;
    }
}

const FCampData& AGMCamp::GetCampData() const
{
    check(mIsInitialized && "Camp should be initialized before providing camp data.");
    return mCampData;
}

const FText& AGMCamp::GetCampName() const
{
    return mCampName;
}
