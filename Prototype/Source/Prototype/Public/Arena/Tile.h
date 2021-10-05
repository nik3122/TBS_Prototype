#pragma once


#include "Tile.generated.h"


struct CompactTile
{
    uint8 X = 0;
    uint8 Y = 0;

    CompactTile() = default;

    CompactTile(const FIntPoint& tile)
        : X(tile.X)
        , Y(tile.Y)
    {}

    CompactTile& operator=(const FIntPoint& tile)
    {
        X = tile.X;
        Y = tile.Y;
        return *this;
    }

    bool operator==(const CompactTile& tile) const
    {
        return X == tile.X && Y == tile.Y;
    }

    bool operator==(const FIntPoint& tile) const
    {
        return X == tile.X && Y == tile.Y;
    }

    bool operator!=(const FIntPoint& tile) const
    {
        return !(*this == tile);
    }

    operator FIntPoint() const
    {
        return { X, Y };
    }
};


USTRUCT(BlueprintType)
struct FTile
{
    GENERATED_BODY()

public:

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 X = -1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 Y = -1;

public:

    static FTile EvenToOdd(const FTile& evenTile);

    static void EvenToOddTiles(const TArray<FTile>& evenTiles, TArray<FTile>& oddTiles);

public:

    FTile() : X(-1), Y(-1) {}

    FTile(int32 _X, int32 _Y) : X(_X), Y(_Y) {}

    FTile(const FIntPoint& intPoint) : X(intPoint.X), Y(intPoint.Y) {}

    operator FIntPoint() { return FIntPoint(X, Y); }

    FTile& operator=(const FIntPoint& intPoint) { X = intPoint.X; Y = intPoint.Y; return *this; }

    bool operator==(const FTile& right) const { return X == right.X && Y == right.Y; }

};


FIntPoint operator+(const FIntPoint& intPoint, const FTile& tile);

bool operator==(const FTile& tile, const FIntPoint& intPoint);
