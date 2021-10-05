

#include "Arena/Tile.h"


FTile FTile::EvenToOdd(const FTile& evenTile)
{
    return FTile(evenTile.X + (FMath::Abs(evenTile.Y) % 2), evenTile.Y);
}

void FTile::EvenToOddTiles(const TArray<FTile>& evenTiles, TArray<FTile>& oddTiles)
{
    oddTiles.Reserve(evenTiles.Num());
    for (const FTile& evenTile : evenTiles)
    {
        oddTiles.Push(EvenToOdd(evenTile));
    }
}

FIntPoint operator+(const FIntPoint& intPoint, const FTile& tile)
{
    return FIntPoint(intPoint.X + tile.X, intPoint.Y + tile.Y);
}

bool operator==(const FTile& tile, const FIntPoint& intPoint)
{
    return tile.X == intPoint.X && tile.Y == intPoint.Y;
}