// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "PassMapData.generated.h"


USTRUCT(BlueprintType)
struct PROTOTYPE_API FTileParameters
{
    GENERATED_USTRUCT_BODY()

public:

    UPROPERTY(EditAnywhere)
    uint8 mStaticLowObstacle : 1;

    UPROPERTY(EditAnywhere)
    uint8 mStaticHighObstacle : 1;

    UPROPERTY(EditAnywhere)
    uint8 mIgnitable : 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool mIsStaticlyOccupied = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool mIsIgnitable = false;

public:

    FTileParameters();

    FTileParameters(bool isStaticlyOccupied,
                    bool isIgnitable);

    bool IsStaticLowObstacle() const;

    bool IsStaticHighObstacle() const;

    bool IsIgnitable() const;
};


/**
    
*/
UCLASS(BlueprintType)
class PROTOTYPE_API UPassMapData : public UObject
{
    GENERATED_BODY()
    
public:

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 mWidth;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 mHeight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float mTileRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FTileParameters> mTiles;

public:

    bool IsTileValid(const FIntPoint& tile) const;

    int32 IndexFromCoord(const FIntPoint& tile) const;

    int32 IndexFromCoord(const FIntPoint& tile,
                         int32 width,
                         int32 height) const;

    const FTileParameters* GetTileParameters(const FIntPoint& tile) const;

    FTileParameters* GetTileParameters(const FIntPoint& tile);

#if WITH_EDITOR

    void PreEditChange(FProperty* propertyAboutToChange) override;

    bool CanEditChange(const FProperty* property) const override;

    void PostEditChangeProperty(FPropertyChangedEvent& propertyChangedEvent) override;

#endif // #if WITH_EDITOR
};
