

#include "Arena/ArenaUtilities.h"
#include "BitmaskOperator.h"
#include "Kismet/KismetSystemLibrary.h"
#include "TileMaps/TileGridCommon.h"


namespace HexTileMapPrecomputed
{
    extern const int32 gPrecomputedHexaAreasCount;

    extern const FIntPoint* GetPrecomputedHexaAreaShifts(int32 areaRadius,
                                                         int32 parityIndex,
                                                         int32& tilesCount);

    extern const int32 gPrecomputedHexaFramesCount;

    extern const FIntPoint* GetPrecomputedHexaFrameShifts(int32 areaRadius,
                                                          int32 parityIndex,
                                                          int32& tilesCount);

    extern const FIntPoint* GetPrecomputedHexaAreaSectoral(int32 areaRadius,
                                                           int32 parityIndex,
                                                           int32& tilesCount);

    extern const FIntPoint* GetPrecomputedHexaSector(int32 areaRadius,
                                                     int32 parityIndex,
                                                     int32 direction,
                                                     int32& firstIndex,
                                                     int32& lastIndex,
                                                     int32& areaSize);
}

//  HexTileMap::

template <uint8 tOccupantSlotsCount>
const float	HexTileGrid<tOccupantSlotsCount>::mRaycastStep = 0.5f;

template <uint8 tOccupantSlotsCount>
const FIntPoint HexTileGrid<tOccupantSlotsCount>::mNeighbourShifts[2][mNeighborsCount] = {
    {
        { 1, 0 }, { 0, 1 }, { -1, 1 }, { -1, 0 }, { -1, -1 }, { 0, -1 },
    },
    {
        { 1, 0 }, { 1, 1 }, { 0, 1 }, { -1, 0 }, { 0, -1 }, { 1, -1 },
    },
};

template <uint8 tOccupantSlotsCount>
FDelegateHandle HexTileGrid<tOccupantSlotsCount>::SetDelegateTileOccupied(const HexTileGrid<tOccupantSlotsCount>::FEventTileOccupation& delegate)
{
    mEventTileOccupied = delegate;
    return delegate.GetHandle();
}

template <uint8 tOccupantSlotsCount>
FDelegateHandle HexTileGrid<tOccupantSlotsCount>::SetDelegateTileReleased(const HexTileGrid<tOccupantSlotsCount>::FEventTileOccupation& delegate)
{
    mEventTileReleased = delegate;
    return delegate.GetHandle();
}

template <uint8 tOccupantSlotsCount>
uint32 HexTileGrid<tOccupantSlotsCount>::GetOccupationSlotIndex(uint8 occupationSlot) const
{
    return static_cast<uint32>(occupationSlot);
}

template <uint8 tOccupantSlotsCount>
void HexTileGrid<tOccupantSlotsCount>::ResetSize(uint32 width,
                                                 uint32 height,
                                                 bool resetData)
{
    check(width > 0 && height > 0);
    uint32 oldWidth = mWidth;
    uint32 oldHeight = mHeight;
    mWidth = width;
    mHeight = height;
    TArray<uint8> oldOccupations(std::move(mTilesOccupations));
    mTilesOccupations.SetNumZeroed(mWidth * mHeight);
    if (resetData == false)
    {
        for (int32 w = 0; w < mWidth; ++w)
        {
            for (int32 h = 0; h < mHeight; ++h)
            {
                if (IsTileValid({ h, w }))
                {
                    int32 oldIndex = IndexFromCoord({ w, h }, oldWidth, oldHeight);
                    mTilesOccupations[IndexFromCoord({ w, h })] = oldOccupations[oldIndex];
                }
            }
        }
        TArray<FIntPoint> tiles;
        mOccupants.GetKeys(tiles);
        for (const FIntPoint& tile : tiles)
        {
            if (IsTileValid(tile))
            {
                mOccupants.Remove(tile);
            }
        }
    }
}

template <uint8 tOccupantSlotsCount>
uint32 HexTileGrid<tOccupantSlotsCount>::GetWidth() const
{
    return mWidth;
}

template <uint8 tOccupantSlotsCount>
uint32 HexTileGrid<tOccupantSlotsCount>::GetHeight() const
{
    return mHeight;
}

template <uint8 tOccupantSlotsCount>
int32 HexTileGrid<tOccupantSlotsCount>::GetTilesNumber() const
{
    return mTilesOccupations.Num();
}

template <uint8 tOccupantSlotsCount>
void HexTileGrid<tOccupantSlotsCount>::SetTileRadius(float tileRadius)
{
    mTileRadius = tileRadius;
}

template <uint8 tOccupantSlotsCount>
float HexTileGrid<tOccupantSlotsCount>::GetTileRadius() const
{
    return mTileRadius;
}

template <uint8 tOccupantSlotsCount>
const TArray<uint8>& HexTileGrid<tOccupantSlotsCount>::GetOccupationData() const
{
    return mTilesOccupations;
}

template <uint8 tOccupantSlotsCount>
FVector HexTileGrid<tOccupantSlotsCount>::GetTilePosition(const FIntPoint& tile) const
{
    return DetermineTilePosition(tile);
}

template <uint8 tOccupantSlotsCount>
FIntPoint HexTileGrid<tOccupantSlotsCount>::GetTileInPosition(const FVector& position) const
{
    FVector pos = position;

    FIntPoint sector;
    sector.X = static_cast<int32>((pos.X  * 2.0f) / GetTileHeight());
    sector.Y = static_cast<int32>((pos.Y - mTileRadius) / GetColumnWidth());

    int32 yIsOdd(sector.Y & 1);
    int32 yIsEven(1 - yIsOdd);
    FIntPoint leftTile(sector.X / 2 - yIsOdd * ((sector.X + 1) & 1), sector.Y);
    FIntPoint rightTile(sector.X / 2 - yIsEven * ((sector.X + 1) & 1), sector.Y + 1);

    FVector leftPos(DetermineTilePosition(leftTile));
    FVector rightPos(DetermineTilePosition(rightTile));
    float toLeft(FMath::Square(pos.X - leftPos.X) + FMath::Square(pos.Y - leftPos.Y));
    float toRight(FMath::Square(pos.X - rightPos.X) + FMath::Square(pos.Y - rightPos.Y));

    return FIntPoint(toLeft > toRight ? rightTile : leftTile);
}

template <uint8 tOccupantSlotsCount>
FIntPoint HexTileGrid<tOccupantSlotsCount>::CoordFromIndex(int32 index) const
{
    return FIntPoint(index % mHeight, index / mHeight);
}

template <uint8 tOccupantSlotsCount>
int32 HexTileGrid<tOccupantSlotsCount>::IndexFromCoord(const FIntPoint& coord) const
{
    return IndexFromCoord(coord, mWidth, mHeight);
}

template <uint8 tOccupantSlotsCount>
int32 HexTileGrid<tOccupantSlotsCount>::IndexFromCoord(const FIntPoint& coord, uint32 width, uint32 height) const
{
    return coord.X + coord.Y * mHeight;
}

template <uint8 tOccupantSlotsCount>
bool HexTileGrid<tOccupantSlotsCount>::IsTileValid(const FIntPoint& tile) const
{
    return tile.X >= 0 && tile.Y >= 0 && tile.X < mHeight && tile.Y < mWidth;
}

template <uint8 tOccupantSlotsCount>
bool HexTileGrid<tOccupantSlotsCount>::IsNeighbour(const FIntPoint& isTile,
                                                   const FIntPoint& ofTile) const
{
    int32 parityIndex = ofTile.Y & 1;
    for (int i = 0; i < mNeighborsCount; ++i)
    {
        if (mNeighbourShifts[parityIndex][i] + ofTile == isTile)
        {
            return true;
        }
    }
    return false;
}

template <uint8 tOccupantSlotsCount>
FIntPoint HexTileGrid<tOccupantSlotsCount>::GetNeighbour(const FIntPoint& tile,
                                                         int32 direction) const
{
    check(IsTileValid(tile));

    FIntPoint neighbour = tile + mNeighbourShifts[tile.Y & 1][direction % mNeighborsCount];
    return neighbour;
}

template <uint8 tOccupantSlotsCount>
int32 HexTileGrid<tOccupantSlotsCount>::GetNeighbourDirection(const FIntPoint& neighbourTile,
                                                              const FIntPoint& targetTile) const
{
    for (int32 direction = 0; IsNeighbourDirectionValid(direction); ++direction)
    {
        FIntPoint neighbourInDirection = mNeighbourShifts[targetTile.Y & 1][direction] + targetTile;
        if (neighbourInDirection == neighbourTile)
        {
            return direction;
        }
    }
    //	Otherwise neighbourTile is not neighbour - return invalid value:
    return -1;
}

template <uint8 tOccupantSlotsCount>
int32 HexTileGrid<tOccupantSlotsCount>::GetDirectionFromVector(const FVector& vector) const
{
    //  ~!~ Vector should be in local coordinates:
    float headingAngle = vector.HeadingAngle();
    int32 headingSector = FMath::TruncToInt(headingAngle * INV_PI * 6.f) + FMath::Sign(headingAngle);
    return (6 + headingSector / 2) % 6;
}

template <uint8 tOccupantSlotsCount>
int32 HexTileGrid<tOccupantSlotsCount>::GetDirectionBetweenTiles(const FIntPoint& fromTile,
                                                                 const FIntPoint& toTile) const
{
    //  ~OPT~ 
    return GetDirectionFromVector(GetTilePosition(toTile) - GetTilePosition(fromTile));
}

template <uint8 tOccupantSlotsCount>
bool HexTileGrid<tOccupantSlotsCount>::CheckNeighborhood(const FIntPoint& targetTile,
                                                         const FIntPoint& neighbourTile,
                                                         int32& direction) const
{
    for (direction = 0; direction < mNeighborsCount; ++direction)
    {
        FIntPoint neighbour = mNeighbourShifts[targetTile.Y & 1][direction] + targetTile;
        if (neighbour == neighbourTile)
        {
            return true;
        }
    }
    //	Otherwise neighbourTile is not neighbour - return invalid value:
    return false;
}

template <uint8 tOccupantSlotsCount>
bool HexTileGrid<tOccupantSlotsCount>::IsNeighbourDirectionValid(int32 direction) const
{
    return direction >= 0 && direction < mNeighborsCount;
}

template <uint8 tOccupantSlotsCount>
void HexTileGrid<tOccupantSlotsCount>::OccupyTile(const FIntPoint& tile,
                                                  uint8 occupationSlot,
                                                  AActor* pOccupant)
{
    mTilesOccupations[IndexFromCoord(tile)] |= MakeBitMask(occupationSlot);
    SetTileOccupant(tile, occupationSlot, pOccupant);
    mEventTileOccupied.ExecuteIfBound(tile, occupationSlot, pOccupant);
}

template <uint8 tOccupantSlotsCount>
void HexTileGrid<tOccupantSlotsCount>::OccupyTile_Safe(const FIntPoint& tile,
                                                       uint8 occupationSlot,
                                                       AActor* pOccupant)
{
    if (IsTileValid(tile) &&
        occupationSlot < tOccupantSlotsCount)
    {
        OccupyTile(tile, occupationSlot, pOccupant);
    }
}

template <uint8 tOccupantSlotsCount>
AActor* HexTileGrid<tOccupantSlotsCount>::FreeTile(const FIntPoint& tile,
                                                   uint8 occupationSlot)
{
    mTilesOccupations[IndexFromCoord(tile)] &= ~MakeBitMask(occupationSlot);
    AActor* pRemovedOccupant = SetTileOccupant(tile, occupationSlot, nullptr);
    mEventTileReleased.ExecuteIfBound(tile, occupationSlot, pRemovedOccupant);
    return pRemovedOccupant;
}

template <uint8 tOccupantSlotsCount>
AActor* HexTileGrid<tOccupantSlotsCount>::FreeTile_Safe(const FIntPoint& tile,
                                                        uint8 occupationSlot)
{
    if (IsTileValid(tile) &&
        occupationSlot < tOccupantSlotsCount)
    {
        return FreeTile(tile, occupationSlot);
    }
    return nullptr;
}

template <uint8 tOccupantSlotsCount>
void HexTileGrid<tOccupantSlotsCount>::ResetAllOccupations(uint8 occupationSlot,
                                                           bool skipNotify)
{
    skipNotify |= !mEventTileReleased.IsBound();
    uint8 occupationMask = MakeBitMask(occupationSlot);
    uint8 invOccupationMask = ~MakeBitMask(occupationSlot);
    for (int32 i = 0; mTilesOccupations.IsValidIndex(i); ++i)
    {
        if (mTilesOccupations[i] & occupationMask)
        {
            FIntPoint tile = CoordFromIndex(i);
            AActor* pOccupant = SetTileOccupant(tile, occupationSlot, nullptr);
            mTilesOccupations[i] &= invOccupationMask;
            if (skipNotify == false)
            {
                mEventTileReleased.Execute(tile, occupationSlot, pOccupant);
            }
        }
    }
}

template <uint8 tOccupantSlotsCount>
bool HexTileGrid<tOccupantSlotsCount>::IsTileFree(const FIntPoint& tile,
                                                  int32 occupationSlotsToCheck,
                                                  bool atLeastOne) const
{
    int32 tileIndex = IndexFromCoord(tile);
    return atLeastOne ?
        (mTilesOccupations[tileIndex] & occupationSlotsToCheck) != occupationSlotsToCheck :
        (mTilesOccupations[tileIndex] & occupationSlotsToCheck) == 0;
}

template <uint8 tOccupantSlotsCount>
bool HexTileGrid<tOccupantSlotsCount>::IsTileFree_Safe(const FIntPoint& tile,
                                                       int32 occupationSlotsToCheck,
                                                       bool atLeastOne) const
{
    return IsTileValid(tile) && IsTileFree(tile, occupationSlotsToCheck, atLeastOne);
}

template <uint8 tOccupantSlotsCount>
bool HexTileGrid<tOccupantSlotsCount>::IsTileFreeAtSlot(const FIntPoint& tile,
                                                        uint8 occupationSlot) const
{
    int32 tileIndex = IndexFromCoord(tile);
    int32 occupationMask = MakeBitMask(occupationSlot);
    return (mTilesOccupations[tileIndex] & occupationMask) == 0;
}

template <uint8 tOccupantSlotsCount>
bool HexTileGrid<tOccupantSlotsCount>::IsTileFreeAtSlot_Safe(const FIntPoint& tile,
                                                             uint8 occupationSlot) const
{
    return IsTileValid(tile) &&
        occupationSlot < tOccupantSlotsCount &&
        IsTileFreeAtSlot(tile, occupationSlot);
}

template <uint8 tOccupantSlotsCount>
AActor* HexTileGrid<tOccupantSlotsCount>::GetOccupant(const FIntPoint& tile,
                                                      uint8 occupationSlot) const
{
    const TileOccupants* pTileOccupants = mOccupants.Find(tile);
    return pTileOccupants != nullptr ? pTileOccupants->mContainer[static_cast<int32>(occupationSlot)].Get() : nullptr;
}

template <uint8 tOccupantSlotsCount>
AActor* HexTileGrid<tOccupantSlotsCount>::GetOccupant_Safe(const FIntPoint& tile,
                                                           uint8 occupationSlot) const
{
    if (IsTileValid(tile) &&
        occupationSlot < tOccupantSlotsCount)
    {
        return GetOccupant(tile, occupationSlot);
    }
    return nullptr;
}

template <uint8 tOccupantSlotsCount>
typename HexTileGrid<tOccupantSlotsCount>::NeighborsContainer HexTileGrid<tOccupantSlotsCount>::Neighbors(const FIntPoint& tile) const
{
    check(IsTileValid(tile) && "HexTileGrid::Neighbors() invalid tile.");

    FIntPoint neihbours[mNeighborsCount];
    int32 neighborsNumber = 0;
    for (int32 i = 0; i < mNeighborsCount; ++i)
    {
        FIntPoint neighbour = mNeighbourShifts[tile.Y & 1][i] + tile;
        if (IsTileValid(neighbour))
        {
            neihbours[neighborsNumber++] = neighbour;
        }
    }

    return NeighborsContainer(*this,
                               neighborsNumber,
                               neihbours);
}

template <uint8 tOccupantSlotsCount>
TArray<FIntPoint> HexTileGrid<tOccupantSlotsCount>::FindPath(const FIntPoint& from,
                                                             const FIntPoint& to,
                                                             int32 occupationSlotsToBeFree,
                                                             int32 occupationSlotsInterruption,
                                                             int32 depthLimit,
                                                             int32 outputLimit,
                                                             bool ignoreTargetTileOccupation)
{
    if (to == from)
    {
        return TArray<FIntPoint>();
    }
    return SearchTask(*this,
                      from,
                      to,
                      occupationSlotsToBeFree,
                      occupationSlotsInterruption,
                      depthLimit,
                      outputLimit,
                      ignoreTargetTileOccupation)();
}

template <uint8 tOccupantSlotsCount>
bool HexTileGrid<tOccupantSlotsCount>::DirectVisibility(const FIntPoint& from,
                                                        const FIntPoint& to,
                                                        int32 occupationSlotsToBeFree,
                                                        float raymarchStepSize,
                                                        FIntPoint& outBlockingTile) const
{
    if (from == to)
    {
        return true;
    }

    check(IsTileValid(from));
    check(IsTileValid(to));

    //      ______
    //     /      \
    //    /   O1___\__Ñ2__
    //    \   |    /   |  \
    //     \__Ñ1__/___O2   \
    //            \        /
    //             \______/
    //
    //  O1.C1.O2.C2 is so called Two-Hex-Rectangle.
    //  If point is inside such rectangle it belongs either to hex O1 or to hex O2.
    //  When raymarching through the tiles, an ambiguous situation may appear:
    //  raymarch point may be placed in between of two tiles. At this moment Two-Hex-Rectangle
    //  may help to make a correct collision check by checking both tiles.

    bool isBlockingTileFound = false;

    float rectSizeY = 1.5f * mTileRadius;
    float rectSizeX = 0.86602540378f * mTileRadius;
    //  Size of the border in between of two neighbor tiles:
    float borderTolerance = mTileRadius * 0.2f;

    //  Normalize positions to make [0; 0] tile have (0; 0) position.
    FVector zeroTilePos = GetTilePosition(FIntPoint(0, 0));
    FVector raymarchStartPos = GetTilePosition(from) - zeroTilePos;
    FVector raymarchEndPos = GetTilePosition(to) -zeroTilePos;
    FVector raymarchVector = raymarchEndPos - raymarchStartPos;
    float raymarchDistance = raymarchVector.Size2D();
    
    for (int32 stepIndex = 1; ; ++stepIndex)
    {
        //  Raymarching point, local coords.
        float relativeDistance = FMath::Min(stepIndex * raymarchStepSize / raymarchDistance, 1.0f);
        if (relativeDistance == 1.0f)
        {
            break;
        }

        FVector point = raymarchStartPos + raymarchVector * relativeDistance;

        int32 rectIndexX = FMath::FloorToInt(point.X / rectSizeX);
        int32 rectIndexY = FMath::FloorToInt(point.Y / rectSizeY);

        FIntPoint tileA;
        tileA.X = (rectIndexX + 1) / 2;
        tileA.Y = ((rectIndexY + 1) / 2) * 2;
        FIntPoint tileB;
        tileB.X = tileA.X + (FMath::Abs(rectIndexX % 2) == 0 ? 0 : -1);
        tileB.Y = tileA.Y + (FMath::Abs(rectIndexY % 2) == 0 ? 1 : -1);

        FVector tilePosA = GetTilePosition(tileA) - zeroTilePos;
        FVector tilePosB = GetTilePosition(tileB) - zeroTilePos;

        float distToA = (tilePosA - point).SizeSquared2D();
        float distToB = (tilePosB - point).SizeSquared2D();

#if WITH_HEX_TILE_GRID_LOG
        UE_LOG(LogTileGrid, VeryVerbose, TEXT("Raymarch step at (%f; %f) | tile A [%i; %i] - (%f; %f) | tile B [%i; %i] - (%f; %f)"),
               point.X + zeroTilePos.X,
               point.Y + zeroTilePos.X,
               tileA.X,
               tileA.Y,
               tilePosA.X,
               tilePosA.Y,
               tileB.X,
               tileB.Y,
               tilePosB.X,
               tilePosB.Y);
#endif // #if WITH_HEX_TILE_GRID_LOG

        bool isTileFreeA = IsTileFree(tileA, occupationSlotsToBeFree);
        bool isTileFreeB = IsTileFree(tileB, occupationSlotsToBeFree);

        if (FMath::IsNearlyEqual(distToA, distToB, borderTolerance))
        {
            if (isTileFreeA == false &&
                isTileFreeB == false)
            {
                isBlockingTileFound = true;
                outBlockingTile = distToA < distToB ? tileA : tileB;
            }
        }
        else if (distToA < distToB)
        {
            if (tileA == to)
            {
                break;
            }
            else if (isTileFreeA == false)
            {
                isBlockingTileFound = true;
                outBlockingTile = tileA;
            }
        }
        else // (distToA > distToB)
        {
            if (tileB == to)
            {
                break;
            }
            else if (isTileFreeB == false)
            {
                isBlockingTileFound = true;
                outBlockingTile = tileB;
            }
        }

        if (isBlockingTileFound)
        {
#if WITH_HEX_TILE_GRID_DEBUG
            if (mDebugWorldContextObject.IsValid())
            {
                UKismetSystemLibrary::DrawDebugSphere(mDebugWorldContextObject.Get(),
                                                      point + zeroTilePos,
                                                      20.f,
                                                      12,
                                                      FLinearColor(1.f, 0.1f, 0.7f),
                                                      WITH_HEX_TILE_GRID_DEBUG_DISPLAY_TIME);
            }
#endif // #if WITH_HEX_TILE_GRID_DEBUG
            break;
        }
        else
        {
#if WITH_HEX_TILE_GRID_DEBUG
            if (mDebugWorldContextObject.IsValid())
            {
                UKismetSystemLibrary::DrawDebugSphere(mDebugWorldContextObject.Get(),
                                                      point + zeroTilePos,
                                                      20.f,
                                                      12,
                                                      FLinearColor::Green,
                                                      WITH_HEX_TILE_GRID_DEBUG_DISPLAY_TIME);
            }
#endif // #if WITH_HEX_TILE_GRID_DEBUG
        }
    }

    return isBlockingTileFound == false;
}

template <uint8 tOccupantSlotsCount>
int32 HexTileGrid<tOccupantSlotsCount>::GetLine(const FIntPoint& from,
                                                const FIntPoint& to,
                                                TArray<FIntPoint>& outTilesOfLine,
                                                float raycastStep)
{
    return 0;
}

template <uint8 tOccupantSlotsCount>
int32 HexTileGrid<tOccupantSlotsCount>::GetReachableTiles(const FIntPoint& centralTile,
                                                          int32 depthLimit,
                                                          int32 occupationSlotsToBeFree,
                                                          TArray<FIntPoint>& outReachableTiles)
{
    struct TileToCheck
    {
        FIntPoint mTile;
        int32 mDepth;
    };
    TArray<TileToCheck> tilesToCheck;
    TArray<FIntPoint> checkedTiles;

    static auto compareTilesToCheck = [] (const TileToCheck& left, const TileToCheck& right) {
        return left.mDepth < right.mDepth;
    };

    tilesToCheck.HeapPush(TileToCheck{ centralTile, 0 }, compareTilesToCheck);

    while (tilesToCheck.Num() > 0)
    {
        TileToCheck tileToCheck;
        tilesToCheck.HeapPop(tileToCheck, compareTilesToCheck);
        checkedTiles.Push(tileToCheck.mTile);

        int32 nextDepth = tileToCheck.mDepth + 1;
        if (nextDepth > depthLimit)
        {
            continue;
        }

        for (auto neighbour : Neighbors(tileToCheck.mTile))
        {
            if (checkedTiles.Contains(neighbour) ||
                IsTileFree(neighbour, occupationSlotsToBeFree) == false)
            {
                continue;
            }

            auto pExistingTile = tilesToCheck.FindByPredicate([&neighbour] (const TileToCheck& checkedTile) {
                return checkedTile.mTile == neighbour;
            });
            if (pExistingTile != nullptr)
            {
                pExistingTile->mDepth = FMath::Min(nextDepth, pExistingTile->mDepth);
                continue;
            }

            tilesToCheck.HeapPush(TileToCheck{ neighbour, nextDepth }, compareTilesToCheck);
        }
    }

    outReachableTiles.Append(checkedTiles);
    return checkedTiles.Num();
}

template <uint8 tOccupantSlotsCount>
int32 HexTileGrid<tOccupantSlotsCount>::GetHexaArea(const FIntPoint& centralTile,
                                                    int32 areaRadius,
                                                    TArray<FIntPoint>& outAreaTiles)
{
    check(areaRadius <= HexTileMapPrecomputed::gPrecomputedHexaAreasCount && areaRadius >= 0 &&
          "Trying to get area of an invalid radius.");
    int32 initialSize = outAreaTiles.Num();
    int32 tileShiftsCount = 0;
    const FIntPoint* pTileShifts = HexTileMapPrecomputed::GetPrecomputedHexaAreaShifts(areaRadius,
                                                                                       centralTile.Y & 1,
                                                                                       tileShiftsCount);
    for (int32 i = 0; i < tileShiftsCount; ++i)
    {
        FIntPoint tile = pTileShifts[i] + centralTile;
        if (IsTileValid(tile))
        {
            outAreaTiles.Push(tile);
        }
    }
    return outAreaTiles.Num() - initialSize;
}

template <uint8 tOccupantSlotsCount>
int32 HexTileGrid<tOccupantSlotsCount>::GetHexaFrame(const FIntPoint& centralTile,
                                                     int32 frameRadius,
                                                     TArray<FIntPoint>& outAreaTiles)
{
    check(frameRadius <= HexTileMapPrecomputed::gPrecomputedHexaFramesCount && frameRadius >= 0 &&
          "Trying to get area of an invalid radius.");
    int32 initialSize = outAreaTiles.Num();
    int32 tileShiftsCount = 0;
    const FIntPoint* pTileShifts = HexTileMapPrecomputed::GetPrecomputedHexaFrameShifts(frameRadius,
                                                                                        centralTile.Y & 1,
                                                                                        tileShiftsCount);
    for (int32 i = 0; i < tileShiftsCount; ++i)
    {
        FIntPoint tile = pTileShifts[i] + centralTile;
        if (IsTileValid(tile))
        {
            outAreaTiles.Push(tile);
        }
    }
    return outAreaTiles.Num() - initialSize;
}

template <uint8 tOccupantSlotsCount>
int32 HexTileGrid<tOccupantSlotsCount>::GetHexaSectors(const FIntPoint& centralTile,
                                                       int32 sectorRadius,
                                                       int32 firstSectorDirection,
                                                       int32 lastSectorDirection,
                                                       TArray<FIntPoint>& outSectorsTiles)
{
    if (firstSectorDirection < 0)
    {
        firstSectorDirection = mNeighborsCount - (-firstSectorDirection % mNeighborsCount);
    }
    firstSectorDirection %= mNeighborsCount;

    if (lastSectorDirection < 0)
    {
        lastSectorDirection = mNeighborsCount - (-lastSectorDirection % mNeighborsCount);
    }
    lastSectorDirection %= mNeighborsCount;

    const FIntPoint* pAreaSectoral = nullptr;
    int32 areaSize = 0;
    int32 firstIndex = 0;
    int32 lastIndex = 0;
    int32 sectorFirstIndex = 0;
    int32 sectorLastIndex = 0;
    HexTileMapPrecomputed::GetPrecomputedHexaSector(
        sectorRadius,
        (centralTile.Y & 1) > 0,
        firstSectorDirection,
        sectorFirstIndex,
        sectorLastIndex,
        areaSize
    );
    firstIndex = sectorFirstIndex;
    pAreaSectoral = HexTileMapPrecomputed::GetPrecomputedHexaSector(
        sectorRadius,
        (centralTile.Y & 1) > 0,
        lastSectorDirection,
        sectorFirstIndex,
        sectorLastIndex,
        areaSize
    );
    lastIndex = sectorLastIndex;

    int32 initialSize = outSectorsTiles.Num();
    if (firstIndex <= lastIndex)
    {
        const FIntPoint* pTileShift = pAreaSectoral + firstIndex;
        const FIntPoint* pLastTileShift = pAreaSectoral + lastIndex + 1;
        while (pTileShift < pLastTileShift)
        {
            FIntPoint tile = *pTileShift + centralTile;
            if (IsTileValid(tile))
            {
                outSectorsTiles.Add(tile);
            }
            ++pTileShift;
        }
    }
    else
    {
        const FIntPoint* pTileShift = pAreaSectoral + firstIndex;
        const FIntPoint* pLastTileShift = pAreaSectoral + areaSize;
        while (pTileShift < pLastTileShift)
        {
            FIntPoint tile = *pTileShift + centralTile;
            if (IsTileValid(tile))
            {
                outSectorsTiles.Add(tile);
            }
            ++pTileShift;
        }
        pTileShift = pAreaSectoral;
        pLastTileShift = pAreaSectoral + lastIndex + 1;
        while (pTileShift < pLastTileShift)
        {
            FIntPoint tile = *pTileShift + centralTile;
            if (IsTileValid(tile))
            {
                outSectorsTiles.Add(tile);
            }
            ++pTileShift;
        }
    }
    return outSectorsTiles.Num() - initialSize;
}

template <uint8 tOccupantSlotsCount>
int32 HexTileGrid<tOccupantSlotsCount>::GetHexaRing(const FIntPoint& centralTile,
                                                    int32 ringRadiusMin,
                                                    int32 ringRadiusMax,
                                                    TArray<FIntPoint>& outRingTiles)
{
    check(ringRadiusMin >= 0 && ringRadiusMin <= HexTileMapPrecomputed::gPrecomputedHexaAreasCount);
    check(ringRadiusMax >= 0 && ringRadiusMax <= HexTileMapPrecomputed::gPrecomputedHexaAreasCount);
    check(ringRadiusMin <= ringRadiusMax);

    int32 initialSize = outRingTiles.Num();

    //  Ring is the difference of two sets of tiles: bigger area - smaller area. Thus 
    //  ~!~HACK~ We store tiles of hexa areas as multiple hexa frames combined in one array in the order of radius incrementation.
    //  Thus difference tile set belongs bigger area and starts at the openListIndex, where smaller area subset finishes.
    int32 smallerAreaSize = 0;
    if (ringRadiusMin > 0)
    {
        HexTileMapPrecomputed::GetPrecomputedHexaAreaShifts(ringRadiusMin - 1, centralTile.Y & 1, smallerAreaSize);
    }
    int32 biggerAreaSize = 0;
    const FIntPoint* pBiggerArea = HexTileMapPrecomputed::GetPrecomputedHexaAreaShifts(ringRadiusMax, centralTile.Y & 1, biggerAreaSize);
    const FIntPoint* pRingStart = pBiggerArea + smallerAreaSize;
    while (pRingStart < pBiggerArea + biggerAreaSize)
    {
        FIntPoint tile = centralTile + *pRingStart;
        if (IsTileValid(tile))
        {
            outRingTiles.Push(tile);
        }
        ++pRingStart;
    }
    return outRingTiles.Num() - initialSize;
}

template <uint8 tOccupantSlotsCount>
template <class Predicate>
void HexTileGrid<tOccupantSlotsCount>::ForEachTileInHexaArea(const FIntPoint& centralTile,
                                                             int32 frameRadius,
                                                             Predicate predicate,
                                                             bool includingCentralTile)
{
    int32 tileShiftsCount = 0;
    const FIntPoint* pTileShifts = HexTileMapPrecomputed::GetPrecomputedHexaAreaShifts(frameRadius,
                                                                                       centralTile.Y & 1,
                                                                                       tileShiftsCount);
    const FIntPoint* pTile = pTileShifts;
    const FIntPoint* pLastTile = pTileShifts + tileShiftsCount;
    while (pTile < pLastTile)
    {
        if (predicate(*pTile + centralTile) == false) return;
        ++pTile;
    }
    if (includingCentralTile)
    {
        predicate(centralTile);
    }
}

template <uint8 tOccupantSlotsCount>
template <class Predicate>
void HexTileGrid<tOccupantSlotsCount>::ForEachTileInHexaFrame(const FIntPoint& centralTile,
                                                              int32 frameRadius,
                                                              Predicate predicate)
{
    int32 tileShiftsCount = 0;
    const FIntPoint* pTileShifts = HexTileMapPrecomputed::GetPrecomputedHexaFrameShifts(frameRadius,
                                                                                        centralTile.Y & 1,
                                                                                        tileShiftsCount);
    const FIntPoint* pTile = pTileShifts;
    const FIntPoint* pLastTile = pTileShifts + tileShiftsCount;
    while (pTile < pLastTile)
    {
        if (predicate(*pTile + centralTile) == false) return;
        ++pTile;
    }
}

template <uint8 tOccupantSlotsCount>
template <class Predicate>
void HexTileGrid<tOccupantSlotsCount>::ForEachTileInSectors(const FIntPoint& centralTile,
                                                            int32 areaRadius,
                                                            int32 firstSectorDirection,
                                                            int32 lastSectorDirection,
                                                            Predicate predicate,
                                                            bool includingCentralTile)
{
    if (firstSectorDirection < 0)
    {
        firstSectorDirection = mNeighborsCount - (-firstSectorDirection % mNeighborsCount);
    }
    firstSectorDirection %= mNeighborsCount;

    if (lastSectorDirection < 0)
    {
        lastSectorDirection = mNeighborsCount - (-lastSectorDirection % mNeighborsCount);
    }
    lastSectorDirection %= mNeighborsCount;

    const FIntPoint* pAreaSectoral = nullptr;
    int32 areaSize = 0;

    int32 firstIndex = 0;
    int32 lastIndex = 0;
    int32 sectorFirstIndex = 0;
    int32 sectorLastIndex = 0;
    HexTileMapPrecomputed::GetPrecomputedHexaSector(
        areaRadius,
        (centralTile.Y & 1) > 0,
        firstSectorDirection,
        sectorFirstIndex,
        sectorLastIndex,
        areaSize
    );
    firstIndex = sectorFirstIndex;
    pAreaSectoral = HexTileMapPrecomputed::GetPrecomputedHexaSector(
        areaRadius,
        (centralTile.Y & 1) > 0,
        lastSectorDirection,
        sectorFirstIndex,
        sectorLastIndex,
        areaSize
    );
    lastIndex = sectorLastIndex;

    if (firstIndex <= lastIndex)
    {
        const FIntPoint* pTile = pAreaSectoral + firstIndex;
        const FIntPoint* pLastTile = pAreaSectoral + lastIndex + 1;
        while (pTile < pLastTile)
        {
            if (predicate(*pTile + centralTile) == false) return;
            ++pTile;
        }
    }
    else
    {
        const FIntPoint* pTile = pAreaSectoral + firstIndex;
        const FIntPoint* pLastTile = pAreaSectoral + areaSize;
        while (pTile < pLastTile)
        {
            if (predicate(*pTile + centralTile) == false) return;
            ++pTile;
        }
        pTile = pAreaSectoral;
        pLastTile = pAreaSectoral + lastIndex + 1;
        while (pTile < pLastTile)
        {
            if (predicate(*pTile + centralTile) == false) return;
            ++pTile;
        }
    }

    if (includingCentralTile)
    {
        predicate(centralTile);
    }
}

template <uint8 tOccupantSlotsCount>
int32 HexTileGrid<tOccupantSlotsCount>::GetRandomFreeTilesInHexaArea(const FIntPoint& centralTile,
                                                                     int32 areaRadius,
                                                                     int32 maxFreeTilesCount,
                                                                     int32 occupationSlotsToBeFree,
                                                                     TArray<FIntPoint>& outRandomFreeTiles)
{
    check(areaRadius <= HexTileMapPrecomputed::gPrecomputedHexaAreasCount && areaRadius > 0 &&
          "Trying to get free tiles in area of an invalid radius.");

    check(maxFreeTilesCount > 0 && "Trying to get 0 random tiles.");

    int32 tileShiftsCount = 0;
    const FIntPoint* pTileShifts = HexTileMapPrecomputed::GetPrecomputedHexaAreaShifts(areaRadius,
                                                                                       centralTile.Y & 1,
                                                                                       tileShiftsCount);

    //	~TODO~ Random iteration without repeats. Create the array of shift indices and shuffle it.

    /*int32 startingIndex = FMath::Rand() % tileShiftsCount;
    int32 coprimeShift = tileShiftsCount / 2 + 1;
    for (int32 i = startingIndex + coprimeShift;
    i != startingIndex && freeTilesCount != maxFreeTilesCount;
    i += coprimeShift)*/

    //	~!~ ~R~ Temporary approach to simulate pseudo random looping through all the shifts:
    int32 freeTilesCount = 0;
    for (int32 i = 0; i < tileShiftsCount && freeTilesCount != maxFreeTilesCount; ++i)
    {
        FIntPoint tile = pTileShifts[i] + centralTile;
        if (IsTileFree_Safe(tile, occupationSlotsToBeFree))
        {
            outRandomFreeTiles.Push(tile);
            ++freeTilesCount;
        }
    }
    return freeTilesCount;
}

template <uint8 tOccupantSlotsCount>
void HexTileGrid<tOccupantSlotsCount>::GetRandomTilesChain(const FIntPoint& chainStartTile,
                                                           int32 chainLength,
                                                           const TArray<FIntPoint>& tilesToIgnore,
                                                           TArray<FIntPoint>& outChainTiles)
{
    outChainTiles.Empty(chainLength);
    outChainTiles.Add(chainStartTile);
    while (outChainTiles.Num() != chainLength)
    {
        FIntPoint tile = GetNeighbour(outChainTiles.Top(), FMath::Rand() % 6);
        if (tilesToIgnore.Contains(tile) == false &&
            IsTileValid(tile))
        {
            outChainTiles.AddUnique(tile);
        }
    }
}

template <uint8 tOccupantSlotsCount>
int32 HexTileGrid<tOccupantSlotsCount>::GetOccupantsInHexaArea(const FIntPoint& centralTile,
                                                               int32 areaRadius,
                                                               uint8 occupationSlot,
                                                               TArray<AActor*>& outOccupants)
{
    check(areaRadius <= HexTileMapPrecomputed::gPrecomputedHexaAreasCount && areaRadius >= 0 &&
          "Trying to get occupants in area of an invalid radius.");
    int32 initialSize = outOccupants.Num();
    int32 tileShiftsCount = 0;
    const FIntPoint* pTileShifts = HexTileMapPrecomputed::GetPrecomputedHexaAreaShifts(areaRadius,
                                                                                       centralTile.Y & 1,
                                                                                       tileShiftsCount);
    for (const FIntPoint* pLastTile = pTileShifts + tileShiftsCount;
         pTileShifts != pLastTile;
         ++pTileShifts)
    {
        const FIntPoint& tile = *pTileShifts + centralTile;
        AActor* pOccupant = GetOccupant_Safe(tile, occupationSlot);
        if (pOccupant != nullptr)
        {
            outOccupants.Push(pOccupant);
        }
    }
    return outOccupants.Num() - initialSize;
}

template <uint8 tOccupantSlotsCount>
int32 HexTileGrid<tOccupantSlotsCount>::GetOccupantsInHexaFrame(const FIntPoint& centralTile,
                                                                int32 frameRadius,
                                                                uint8 occupationSlot,
                                                                TArray<AActor*>& outOccupants)
{
    int32 initialSize = outOccupants.Num();
    int32 tileShiftsCount = 0;
    const FIntPoint* pTileShifts = HexTileMapPrecomputed::GetPrecomputedHexaFrameShifts(frameRadius,
                                                                                        centralTile.Y & 1,
                                                                                        tileShiftsCount);
    for (const FIntPoint* pLastTile = pTileShifts + tileShiftsCount;
         pTileShifts != pLastTile;
         ++pTileShifts)
    {
        const FIntPoint& tile = *pTileShifts + centralTile;
        AActor* pOccupant = GetOccupant_Safe(tile, occupationSlot);
        if (pOccupant != nullptr)
        {
            outOccupants.Push(pOccupant);
        }
    }
    return outOccupants.Num() - initialSize;
}

template <uint8 tOccupantSlotsCount>
int32 HexTileGrid<tOccupantSlotsCount>::GetOccupantsInSectors(const FIntPoint& centralTile,
                                                              int32 areaRadius,
                                                              int32 firstSectorDirection,
                                                              int32 lastSectorDirection,
                                                              uint8 occupationSlot,
                                                              TArray<AActor*>& outOccupants)
{
    if (firstSectorDirection < 0)
    {
        firstSectorDirection = mNeighborsCount - (-firstSectorDirection % mNeighborsCount);
    }
    firstSectorDirection %= mNeighborsCount;

    if (lastSectorDirection < 0)
    {
        lastSectorDirection = mNeighborsCount - (-lastSectorDirection % mNeighborsCount);
    }
    lastSectorDirection %= mNeighborsCount;

    int32 initialSize = outOccupants.Num();
    const FIntPoint* pAreaSectoral = nullptr;
    int32 areaSize = 0;

    int32 firstIndex = 0;
    int32 lastIndex = 0;
    int32 sectorFirstIndex = 0;
    int32 sectorLastIndex = 0;
    pAreaSectoral = HexTileMapPrecomputed::GetPrecomputedHexaSector(
        areaRadius,
        (centralTile.Y & 1) > 0,
        firstSectorDirection,
        sectorFirstIndex,
        sectorLastIndex,
        areaSize
    );
    firstIndex = sectorFirstIndex;
    pAreaSectoral = HexTileMapPrecomputed::GetPrecomputedHexaSector(
        areaRadius,
        (centralTile.Y & 1) > 0,
        lastSectorDirection,
        sectorFirstIndex,
        sectorLastIndex,
        areaSize
    );
    lastIndex = sectorLastIndex;

    if (firstIndex <= lastIndex)
    {
        for (int32 i = firstIndex; i <= lastIndex; ++i)
        {
            AActor* pOccupant = GetOccupant_Safe(pAreaSectoral[i] + centralTile, occupationSlot);
            if (pOccupant != nullptr)
            {
                outOccupants.Push(pOccupant);
            }
        }
    }
    else
    {
        for (int32 i = firstIndex; i < areaSize; ++i)
        {
            AActor* pOccupant = GetOccupant_Safe(pAreaSectoral[i] + centralTile, occupationSlot);
            if (pOccupant != nullptr)
            {
                outOccupants.Push(pOccupant);
            }
        }
        for (int32 i = 0; i <= lastIndex; ++i)
        {
            AActor* pOccupant = GetOccupant_Safe(pAreaSectoral[i] + centralTile, occupationSlot);
            if (pOccupant != nullptr)
            {
                outOccupants.Push(pOccupant);
            }
        }
    }

    return outOccupants.Num() - initialSize;
}

#if WITH_HEX_TILE_GRID_DEBUG

template <uint8 tOccupantSlotsCount>
void HexTileGrid<tOccupantSlotsCount>::DebugSetWorldContextObject(UObject* worldContextObject)
{
    mDebugWorldContextObject = worldContextObject;
}

#endif // #if WITH_HEX_TILE_GRID_DEBUG

template <uint8 tOccupantSlotsCount>
float HexTileGrid<tOccupantSlotsCount>::GetTileHeight() const
{
    //	The height of a tile equals doubled radius multiplied cos(30):
    return mTileRadius * 2.0f * 0.8660254f;
}

template <uint8 tOccupantSlotsCount>
float HexTileGrid<tOccupantSlotsCount>::GetColumnWidth() const
{
    //	The column width equals tile radius multiplied 1.5:
    return mTileRadius * 1.5f;
}

template <uint8 tOccupantSlotsCount>
FVector HexTileGrid<tOccupantSlotsCount>::DetermineTilePosition(const FIntPoint& tile) const
{
    return FVector(((float)tile.X + 0.5f * (float)(1 + (tile.Y & 1))) * GetTileHeight(),
                   (float)tile.Y * GetColumnWidth() + mTileRadius,
                   0.0f);
}

template <uint8 tOccupantSlotsCount>
AActor* HexTileGrid<tOccupantSlotsCount>::SetTileOccupant(const FIntPoint& tile,
                                                          uint8 occupationSlot,
                                                          AActor* pNewOccupant)
{
    AActor* pOldOccupant = nullptr;
    int32 slotIndex = static_cast<int32>(occupationSlot);
    TileOccupants* pTileOccupants = mOccupants.Find(tile);
    if (pTileOccupants != nullptr)
    {

        pOldOccupant = pTileOccupants->mContainer[slotIndex].Get();
        pTileOccupants->mContainer[slotIndex] = pNewOccupant;

        if (pNewOccupant == nullptr)
        {
            //	Check if there is at least one occupant at this tile:
            bool shouldBeRemoved = true;
            for (TWeakObjectPtr<AActor> pOccupant : pTileOccupants->mContainer)
            {
                if (pOccupant != nullptr)
                {
                    shouldBeRemoved = false;
                    break;
                }
            }
            //  If no occupants was detected - remove it from the map:
            if (shouldBeRemoved)
            {
                mOccupants.Remove(tile);
            }
        }
    }
    else if (pNewOccupant != nullptr)
    {
        mOccupants.Add(tile).mContainer[slotIndex] = pNewOccupant;
    }
    return pOldOccupant;
}

template <uint8 tOccupantSlotsCount>
int32 HexTileGrid<tOccupantSlotsCount>::GetDistance(const FIntPoint& tileA,
                                                    const FIntPoint& tileB) const
{
    FIntVector cubeTileA;
    cubeTileA.X = tileA.Y;
    cubeTileA.Z = tileA.X - (tileA.Y - (tileA.Y & 1)) / 2;
    cubeTileA.Y = -cubeTileA.X - cubeTileA.Z;
    FIntVector cubeTileB;
    cubeTileB.X = tileB.Y;
    cubeTileB.Z = tileB.X - (tileB.Y - (tileB.Y & 1)) / 2;
    cubeTileB.Y = -cubeTileB.X - cubeTileB.Z;
    int32 distance = (FMath::Abs(cubeTileA.X - cubeTileB.X) + FMath::Abs(cubeTileA.Y - cubeTileB.Y) + FMath::Abs(cubeTileA.Z - cubeTileB.Z)) / 2;
    return distance;
}

template <uint8 tOccupantSlotsCount>
HexTileGrid<tOccupantSlotsCount>::SearchTask::SearchTask(const HexTileGrid<tOccupantSlotsCount>& hexTileMap,
                                                         const FIntPoint& from,
                                                         const FIntPoint& to,
                                                         int32 occupationSlotsToBeFree,
                                                         int32 occupationSlotsInterruption,
                                                         uint32 depthLimit,
                                                         uint32 outputLimit,
                                                         bool ignoreTargetTileOccupation)
    : mTileMap(hexTileMap)
    , mFrom(from)
    , mTo(to)
    , mOccupationSlotsToBeFree(occupationSlotsToBeFree)
    , mOccupationSlotsInterruption(occupationSlotsInterruption)
    , mOutputLimit(outputLimit)
    , mDepthLimit(depthLimit)
    , mIgnoreTargetTileOccupation(ignoreTargetTileOccupation)
{}

template <uint8 tOccupantSlotsCount>
TArray<FIntPoint> HexTileGrid<tOccupantSlotsCount>::SearchTask::operator()()
{
    static auto compareChecked = [] (const OpenTile& left, const OpenTile& right) {
        return left.mTotalCost < right.mTotalCost;
    };

    //  ~!~ Swap From and To tiles to get rid of reverse in the end:
    FIntPoint start = mTo;
    FIntPoint finish = mFrom;

    //	Start searching from the neighbors of the first tile (it doesn't matter whether From-tile occupied or not)
    mOpenList.HeapPush(OpenTile{ start, start, mTileMap.GetDistance(start, finish), 0 }, compareChecked);

    CompactTile lastTile;
    uint32 pathLength = 0;

    while (pathLength == 0 && mOpenList.Num() > 0)
    {
        OpenTile openTile;
        //	Get tile with the lowest weight:
        mOpenList.HeapPop(openTile, compareChecked, false);

        //	Push it to the closed list as checked:
        ClosedTile& closedTile = mClosedList[mClosedList.AddUnique(ClosedTile{ openTile.mTile })];
        closedTile.mParent = openTile.mParent;
        closedTile.mTotalCost = openTile.mTotalCost;

        //	If the depth of search has reached path's length limit - discard this tile:
        uint32 nextDepth = openTile.mDepth + 1;
        if (nextDepth > mDepthLimit)
        {
            continue;
        };

        //  For all valid neighbors:
        for (int32 direction = 0; direction < 6; ++direction)
        {
            FIntPoint neighbour = mTileMap.GetNeighbour({ openTile.mTile.X, openTile.mTile.Y }, direction);
            if (mTileMap.IsTileValid(neighbour) == false)
            {
                continue;
            }
            else if (neighbour == finish)
            {
                //  Remember current tile to be the last checked tile and finish searching:
                lastTile = openTile.mTile;
                pathLength = nextDepth;
                break;
            }
            //  If the neighbour is occupied - skip it:
            else if (mTileMap.IsTileFree(neighbour, mOccupationSlotsToBeFree) == false)
            {
                continue;
            }

            //  Calculate the weight for neighbour:
            int32 neighbourCostToGoal = mTileMap.GetDistance(neighbour, finish);
            int32 neighbourTotalCost = neighbourCostToGoal + nextDepth;

            //  Check if the neighbour already exists in open list and if it with the total cost higher than counted - update it:
            int32 openListIndex = mOpenList.FindLastByPredicate([&neighbour] (const OpenTile& checkedTile) {
                return checkedTile.mTile == neighbour;
            });
            if (openListIndex != INDEX_NONE)
            {
                if (neighbourTotalCost >= mOpenList[openListIndex].mTotalCost)
                {
                    continue;
                }
                mOpenList.HeapRemoveAt(openListIndex, compareChecked, false);
            }

            int32 closedListIndex = mClosedList.Find(ClosedTile{ neighbour });
            if (closedListIndex != INDEX_NONE)
            {
                if (mClosedList[closedListIndex].mTotalCost > neighbourTotalCost)
                {
                    mClosedList[closedListIndex].mParent = openTile.mParent;
                    mClosedList[closedListIndex].mTotalCost = neighbourTotalCost;
                }
                continue;
            };

            mOpenList.HeapPush(OpenTile{ neighbour, openTile.mTile, neighbourTotalCost, nextDepth }, compareChecked);
        }
    }

    TArray<FIntPoint> path;

    //	If the path was found - try to save it to an array.
    //	~?~ As in the very beginning we swapped From- and To- tiles, now the path is going in the forward order.
    if (pathLength > 0)
    {
        path.Reserve(pathLength + 1);

        //	Fill the path with found tiles by rolling back from the last tile to the very beginning:
        while (lastTile != start)
        {
            path.Push(lastTile);
            lastTile = mClosedList[mClosedList.Find(ClosedTile{ lastTile })].mParent;
        }

        // The path doesn't contain the start tile. So include it into the path:
        if (mIgnoreTargetTileOccupation ||
            mTileMap.IsTileFree(start, mOccupationSlotsToBeFree))
        {
            path.Push(start);
        }

        // As the path is filled from To-tile to From-tile - it is needed to be reverted:
        //Algo::Reverse(path);
        uint32 movementInterruptionIndex = 0;
        for (; movementInterruptionIndex < static_cast<uint32>(path.Num());)
        {
            auto& tile = path[movementInterruptionIndex++];
            if (mTileMap.IsTileFree(tile, mOccupationSlotsInterruption) == false)
            {
                break;
            }
        }

        // If an output limit it exceeded then cut the path:
        uint32 cutOffIndex = FMath::Min(movementInterruptionIndex, mOutputLimit);
        if (cutOffIndex < static_cast<uint32>(path.Num()))
        {
            path.RemoveAt(cutOffIndex, path.Num() - cutOffIndex, false);
        }
    }

    return path;
}


//////////////////////////////////////////////////////////////////////////
//	NeighborsContainer
//////////////////////////////////////////////////////////////////////////

template <uint8 tOccupantSlotsCount>
typename HexTileGrid<tOccupantSlotsCount>::NeighborsContainer::Iterator& HexTileGrid<tOccupantSlotsCount>::NeighborsContainer::Iterator::operator++(int)
{
    ++mIndex;
    return *this;
}

template <uint8 tOccupantSlotsCount>
typename HexTileGrid<tOccupantSlotsCount>::NeighborsContainer::Iterator& HexTileGrid<tOccupantSlotsCount>::NeighborsContainer::Iterator::operator++()
{
    ++mIndex;
    return *this;
}

template <uint8 tOccupantSlotsCount>
bool HexTileGrid<tOccupantSlotsCount>::NeighborsContainer::Iterator::operator==(const Iterator& right) const
{
    return mIndex == right.mIndex ||
        (mIndex >= mNeighbourShifts.mCount &&
         right.mIndex >= mNeighbourShifts.mCount);
}

template <uint8 tOccupantSlotsCount>
bool HexTileGrid<tOccupantSlotsCount>::NeighborsContainer::Iterator::operator!=(const Iterator& right) const
{
    return !(*this == right);
}

template <uint8 tOccupantSlotsCount>
FIntPoint HexTileGrid<tOccupantSlotsCount>::NeighborsContainer::Iterator::operator*()
{
    check(mIndex < mNeighbourShifts.mCount);
    return mNeighbourShifts.mNeighbourShifts[mIndex];
}

template <uint8 tOccupantSlotsCount>
const FIntPoint& HexTileGrid<tOccupantSlotsCount>::NeighborsContainer::Iterator::operator*() const
{
    check(mIndex < mNeighbourShifts.mCount);
    return mNeighbourShifts.mNeighbourShifts[mIndex];
}

template <uint8 tOccupantSlotsCount>
HexTileGrid<tOccupantSlotsCount>::NeighborsContainer::Iterator::Iterator(NeighborsContainer& neighbors):
    mNeighbourShifts(neighbors),
    mIndex(0) {}

template <uint8 tOccupantSlotsCount>
HexTileGrid<tOccupantSlotsCount>::NeighborsContainer::Iterator::Iterator(NeighborsContainer& neighbors, bool):
    mNeighbourShifts(neighbors),
    mIndex(HexTileGrid<tOccupantSlotsCount>::mNeighborsCount) {}

template <uint8 tOccupantSlotsCount>
typename HexTileGrid<tOccupantSlotsCount>::NeighborsContainer::Iterator HexTileGrid<tOccupantSlotsCount>::NeighborsContainer::begin()
{
    return mNeighborsCount > 0 ? Iterator(*this) : Iterator(*this, false);
}

template <uint8 tOccupantSlotsCount>
typename HexTileGrid<tOccupantSlotsCount>::NeighborsContainer::Iterator HexTileGrid<tOccupantSlotsCount>::NeighborsContainer::end()
{
    return Iterator(*this, false);
}

template <uint8 tOccupantSlotsCount>
HexTileGrid<tOccupantSlotsCount>::NeighborsContainer::NeighborsContainer(const HexTileGrid& passMap,
                                                                           int32 neighborsCount,
                                                                           FIntPoint neighbors[6])
    : mTileMap(passMap)
    , mCount(neighborsCount)
{
    for (int32 i = 0; i < HexTileGrid<tOccupantSlotsCount>::mNeighborsCount; ++i)
    {
        mNeighbourShifts[i] = neighbors[i];
    }
}
