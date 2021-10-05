

#include "TileMaps/HexTilePattern.h"


#pragma optimize ("", off)
FIntVector HexTileCoords2Dto3D(const FIntPoint& coords2D)
{
    FIntVector coords3D = { 0, 0, 0 };
    coords3D.X = coords2D.X - (coords2D.Y - (int32)(coords2D.Y < 0)) / 2;
    coords3D.Z = coords2D.Y;
    coords3D.Y = coords3D.X + coords3D.Z;
    return coords3D;
}

FIntPoint HexTileCoords3Dto2D(const FIntVector& coords3D)
{
    check(coords3D.Y == coords3D.X + coords3D.Z);
    FIntPoint coords2D = { 0, 0 };
    coords2D.Y = coords3D.Z;
    coords2D.X = coords3D.X + (coords2D.Y - (int32)(coords2D.Y < 0)) / 2;
    return coords2D;
}


void UHexTilePattern::PostInitProperties()
{
    Super::PostInitProperties();

    UpdateCachedPatterns();
}

TArray<FIntPoint> UHexTilePattern::GetPatternTiles(const FIntPoint& originTile,
                                                   int32 tileDirection) const
{
    tileDirection = ((tileDirection % 6) + 6) % 6;
    if (mAllowCaching)
    {
        int32 cacheIndex = GetCacheIndex(originTile, tileDirection);
        TArray<FIntPoint> patternTiles = mCachedPatterns[cacheIndex].mTileShifts;
        for (FIntPoint& tile : patternTiles)
        {
            tile += originTile;
        }
        return patternTiles;
    }
    else
    {
        return GeneratePattern(originTile, tileDirection, originTile.Y % 2);
    }
}


#if WITH_EDITOR

void UHexTilePattern::PostEditChangeProperty(FPropertyChangedEvent& propertyChangedEvent)
{
    FName propertyName = propertyChangedEvent.MemberProperty->GetFName();

    bool isCacheDirty = false;

    if (propertyName == GET_MEMBER_NAME_CHECKED(UHexTilePattern, mAllowCaching))
    {
        if (mAllowCaching == false)
        {
            mCachedPatterns.Empty();
        }
    }
    else if (propertyName == GET_MEMBER_NAME_CHECKED(UHexTilePattern, mIsOriented))
    {
        isCacheDirty = true;
    }
    else if (propertyName == GET_MEMBER_NAME_CHECKED(UHexTilePattern, mExamplePattern))
    {
        isCacheDirty = true;
    }

    if (isCacheDirty)
    {
        UpdateCachedPatterns();
    }

    Super::PostEditChangeProperty(propertyChangedEvent);
}

#endif // #if WITH_EDITOR

TArray<FIntPoint> UHexTilePattern::GeneratePattern(const FIntPoint& originTile,
                                                   int32 tileDirection,
                                                   int32 overrideTileParity) const
{
    check(tileDirection >= 0 && tileDirection < 6);
    TArray<FIntPoint> pattern(mExamplePattern);
    if (mIsOriented)
    {
        for (FIntPoint& tileShift : pattern)
        {
            //  Rotate tile using 3D coords:
            FIntVector coord3D = HexTileCoords2Dto3D(tileShift);
            FIntVector rotCoord3D;
            switch (tileDirection)
            {
            case 0:
                rotCoord3D = coord3D;
                break;

            case 1:
                rotCoord3D.X = -coord3D.Z;
                rotCoord3D.Y = coord3D.X;
                rotCoord3D.Z = coord3D.Y;
                break;

            case 2:
                rotCoord3D.X = -coord3D.Y;
                rotCoord3D.Y = -coord3D.Z;
                rotCoord3D.Z = coord3D.X;
                break;

            case 3:
                rotCoord3D.X = -coord3D.X;
                rotCoord3D.Y = -coord3D.Y;
                rotCoord3D.Z = -coord3D.Z;
                break;

            case 4:
                rotCoord3D.X = coord3D.Z;
                rotCoord3D.Y = -coord3D.X;
                rotCoord3D.Z = -coord3D.Y;
                break;

            case 5:
                rotCoord3D.X = coord3D.Y;
                rotCoord3D.Y = coord3D.Z;
                rotCoord3D.Z = -coord3D.X;
                break;
            }
            tileShift = HexTileCoords3Dto2D(rotCoord3D);

            //  Example pattern is for even origin tile.
            //  Adjust X shifts for odd origin tiles.
            tileShift.X += overrideTileParity == 0 ? 0 : FMath::Abs(tileShift.Y) % 2;
            tileShift += originTile;
        }
    }
    else
    {
        for (FIntPoint& tileShift : pattern)
        {
            //  Example pattern is for even origin tile.
            //  Adjust X shifts for odd origin tiles.
            tileShift.X += overrideTileParity == 0 ? 0 : FMath::Abs(tileShift.Y) % 2;
            tileShift += originTile;
        }
    }
    return pattern;
}

int32 UHexTilePattern::GetCacheIndex(const FIntPoint& originTile,
                                     int32 tileDirection) const
{
    check(tileDirection >= 0 && tileDirection < 6);
    int32 tileParity = originTile.Y & 1;
    return mIsOriented ? tileParity * 6 + tileDirection : tileParity;
}

void UHexTilePattern::UpdateCachedPatterns()
{
    if (mExamplePattern.Num() == 0)
    {
        mCachedPatterns.Empty();
        return;
    }
    if (mIsOriented)
    {
        mCachedPatterns.SetNum(12);

        //  First 6 caches contain patterns for even origin tile, from 0 direction to 5 direction.
        //  Next 6 caches contain patterns for odd origin tile, from 0 direction to 5 direction.
        for (int32 cacheIndex = 0; cacheIndex < 12; ++cacheIndex)
        {
            int32 tileDirection = cacheIndex % 6;
            int32 tileParity = cacheIndex >= 6;
            FHexTilePatternCache& patternCache = mCachedPatterns[cacheIndex];
            patternCache.mTileShifts = GeneratePattern(FIntPoint(0, 0), tileDirection, tileParity);
        }
    }
    else
    {
        mCachedPatterns.SetNum(2);

        //  Cache 0 contains pattern for even origin tile.
        {
            FHexTilePatternCache& patternCache = mCachedPatterns[0];
            patternCache.mTileShifts = GeneratePattern(FIntPoint(0, 0), 0, 0);
        }

        //  Cache 1 contains pattern for odd origin tile.
        {
            FHexTilePatternCache& patternCache = mCachedPatterns[1];
            patternCache.mTileShifts = GeneratePattern(FIntPoint(0, 0), 0, 1);
        }
    }
}

#pragma optimize ("", on)