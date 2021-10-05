// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "EditorPassMapSaver.generated.h"


USTRUCT(BlueprintType)
struct CUSTOMEDITORS_API FTileDataToSave
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintReadWrite)
	bool mIsStaticlyOccupied;

	UPROPERTY(BlueprintReadWrite)
	bool mIsIgnitable;
};


/**
 * 
 */
UCLASS(Blueprintable)
class CUSTOMEDITORS_API UEditorPassMapSaver : public UObject
{
	GENERATED_BODY()
	
public:

	UFUNCTION(BlueprintCallable, Category = "FwSL|Editor")
	bool SaveToFile(const FString& filePath,
					int32 widthInTiles,
					int32 heightInTiles,
					const TArray<FTileDataToSave>& tilesDataToSave);
	
	
};
