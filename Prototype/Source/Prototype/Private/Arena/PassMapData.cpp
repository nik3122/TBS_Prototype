// Fill out your copyright notice in the Description page of Project Settings.

#include "Arena/PassMapData.h"


FTileParameters::FTileParameters()
    : mStaticLowObstacle(0)
    , mStaticHighObstacle(0)
    , mIgnitable(0)
    , mIsStaticlyOccupied(false)
    , mIsIgnitable(false)
{}

FTileParameters::FTileParameters(bool isStaticlyOccupied,
                                 bool isIgnitable):
    mIsStaticlyOccupied(isStaticlyOccupied),
    mIsIgnitable(isIgnitable)
{}

bool FTileParameters::IsStaticLowObstacle() const
{
    return mStaticLowObstacle;
}

bool FTileParameters::IsStaticHighObstacle() const
{
    return mStaticHighObstacle;
}

bool FTileParameters::IsIgnitable() const
{
    return mIsIgnitable || mIgnitable;
}

bool UPassMapData::IsTileValid(const FIntPoint& tile) const
{
    return tile.X >= 0 && tile.X < mHeight && tile.Y >= 0 && tile.Y < mWidth;
}

int32 UPassMapData::IndexFromCoord(const FIntPoint& tile) const
{
    return IndexFromCoord(tile, mWidth, mHeight);
}

int32 UPassMapData::IndexFromCoord(const FIntPoint& tile,
                                   int32 width,
                                   int32 height) const
{
    return tile.Y * height + tile.X;
}

const FTileParameters* UPassMapData::GetTileParameters(const FIntPoint& tile) const
{
    return IsTileValid(tile) ? &mTiles[IndexFromCoord(tile)] : nullptr;
}

FTileParameters* UPassMapData::GetTileParameters(const FIntPoint& tile)
{
    return IsTileValid(tile) ? &mTiles[IndexFromCoord(tile)] : nullptr;
}

#if WITH_EDITOR

void UPassMapData::PreEditChange(FProperty* propertyAboutToChange)
{
    Super::PreEditChange(propertyAboutToChange);
}

bool UPassMapData::CanEditChange(const FProperty* property) const
{
    return Super::CanEditChange(property);
}

void UPassMapData::PostEditChangeProperty(FPropertyChangedEvent& propertyChangedEvent)
{
    Super::PostEditChangeProperty(propertyChangedEvent);
}

#endif // #if WITH_EDITOR