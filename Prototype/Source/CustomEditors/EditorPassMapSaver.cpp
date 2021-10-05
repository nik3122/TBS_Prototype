// Fill out your copyright notice in the Description page of Project Settings.

#include "EditorPassMapSaver.h"
#include "XmlFile.h"
#include "Misc/Paths.h"
#include <tinyxml2.h>
#include <string>


bool UEditorPassMapSaver::SaveToFile(const FString& filePath,
									 int32 widthInTiles,
									 int32 heightInTiles,
									 const TArray<FTileDataToSave>& tilesDataToSave)
{
	tinyxml2::XMLDocument doc;

	auto pNode_PassMap = doc.NewElement("passmap");
	doc.InsertEndChild(pNode_PassMap);
	pNode_PassMap->SetAttribute("width", widthInTiles);
	pNode_PassMap->SetAttribute("height", heightInTiles);

	auto pNode_Tiles = doc.NewElement("tiles");
	pNode_PassMap->InsertFirstChild(pNode_Tiles);

	TArray<FTCHARToUTF8> utf8_tilesDataValue;
	FString tileDataValue;
	for (int32 tileIndex = 0; tileIndex < tilesDataToSave.Num(); ++tileIndex)
	{
		auto pNode_Tile = doc.NewElement("t");
		pNode_Tiles->InsertEndChild(pNode_Tile);

		int32 x = tileIndex % heightInTiles;
		int32 y = tileIndex / heightInTiles;
		const auto& tileData = tilesDataToSave[tileIndex];
		tileDataValue = FString::Printf(TEXT("x%iy%io%ii%i"), x, y, (int32)tileData.mIsStaticlyOccupied, (int32)tileData.mIsIgnitable);
		utf8_tilesDataValue.Emplace(*tileDataValue);
		auto xmlText = doc.NewText(utf8_tilesDataValue.Last().Get());

		pNode_Tile->SetText(utf8_tilesDataValue.Last().Get());
	}

	FString fullFilePath = FPaths::Combine(FPaths::ProjectDir(), filePath);

	auto utf8_filePath = TCHAR_TO_UTF8(*fullFilePath);
	auto success = doc.SaveFile(utf8_filePath);

#if UE_BUILD_DEBUG
	if (success == tinyxml2::XML_SUCCESS)
	{
		UE_LOG(LogTemp, Display, TEXT("TinyXML2: saving SUCCEEDED"));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("TinyXML2: saving FAILED. Error code: %i"), (int32)success);
	}
#endif UE_BUILD_DEBUG

	return false;
}
