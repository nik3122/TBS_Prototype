#pragma once


#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "HexTilePattern.generated.h"


USTRUCT(BlueprintType)
struct FHexTilePatternCache
{
    GENERATED_BODY()

public:

    UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
    TArray<FIntPoint> mTileShifts;
};


/**
 *  Pattern doesn't contain a particular tile coordinates.
 *  Instead it contains a tile shifts - coordinates difference between pattern tile
 *  and origin tile passed dynamically.
 */
UCLASS()
class UHexTilePattern
    : public UDataAsset
{
    GENERATED_BODY()

public:

    void PostInitProperties() override;

    TArray<FIntPoint> GetPatternTiles(const FIntPoint& originTile,
                                      int32 tileDirection) const;

#if WITH_EDITOR

    void PostEditChangeProperty(FPropertyChangedEvent& propertyChangedEvent) override;

#endif // #if WITH_EDITOR

protected:

    /**
     *  Set of tiles relative to an origin tile. Origin tile is supposed to have even Y coordinate.
     *  Pattern is oriented toward tile grid's X axis.
     */
    UPROPERTY(EditAnywhere)
    TArray<FIntPoint> mExamplePattern;

    /**
     *  Determines if rotation by tile direction must be applied for pattern.
     */
    UPROPERTY(EditAnywhere)
    bool mIsOriented = false;

    /**
     *  If true, all possible permutations are cached.
     */
    UPROPERTY(EditAnywhere)
    bool mAllowCaching = true;

    /*
     *  
     */
    UPROPERTY(VisibleAnywhere)
    TArray<FHexTilePatternCache> mCachedPatterns;

private:

    TArray<FIntPoint> GeneratePattern(const FIntPoint& originTile,
                                      int32 tileDirection,
                                      int32 overrideTileParity) const;

    int32 GetCacheIndex(const FIntPoint& originTile,
                        int32 tileDirection) const;

    void UpdateCachedPatterns();
};
