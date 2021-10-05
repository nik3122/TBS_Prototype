

#include "XmlCallback_PassMapData.h"


FXmlCallback_PassMapData::FXmlCallback_PassMapData():
	mParsingState(ParsingState::ParsingState_Uninitialized)
{}

bool FXmlCallback_PassMapData::LoadPassMapData(UPassMapData* pPassMapData,
											   FString& xmlData,
											   bool isFilePath,
											   FFeedbackContext* pFeedbackContext)
{
	mpPassMapData = pPassMapData;

	mpPassMapData->mWidth = 0;
	mpPassMapData->mHeight = 0;
	mpPassMapData->mTileRadius = 100.0f;
	mpPassMapData->mTiles.Empty();

	mParsingState = ParsingState::ParsingState_Uninitialized;

	FText errorMessage;
	int32 errorLineNumber;
	if (FFastXml::ParseXmlFile(this,
							   isFilePath ? *xmlData : nullptr,
							   isFilePath ? nullptr : xmlData.GetCharArray().GetData(),
							   pFeedbackContext,
							   true,
							   true,
							   errorMessage,
							   errorLineNumber))
	{

		return true;
	}

	return false;
}

bool FXmlCallback_PassMapData::ProcessXmlDeclaration(const TCHAR* pElementData,
													 int32 XmlFileLineNumber)
{
	return true;
}

bool FXmlCallback_PassMapData::ProcessElement(const TCHAR* pElementName,
											  const TCHAR* pElementData,
											  int32 XmlFileLineNumber)
{
	if (IsInitialized() == false)
	{
		if (FCString::Stricmp(pElementName, sElem_PassMap) == 0)
		{
			mParsingState = ParsingState::ParsingState_PassMap;
		}
	}
	else
	{
		if (mParsingState == ParsingState::ParsingState_PassMap &&
			FCString::Stricmp(pElementName, sElem_Tiles) == 0)
		{
			mParsingState = ParsingState::ParsingState_Tiles;
		}
		else if(mParsingState == ParsingState::ParsingState_Tiles &&
				FCString::Stricmp(pElementName, sElem_Tile) == 0)
		{
			//	Parse next format: x[coord]y[coord]o[1/0]i[1/0]
			
			const TCHAR* pX = FCString::Strchr(pElementData, TCHAR('x')) + 1;
			const int32 x = FCString::Atoi(pX);
			const TCHAR* pY = FCString::Strchr(pElementData, TCHAR('y')) + 1;
			const int32 y = FCString::Atoi(pY);

			if (mpPassMapData->IsTileValid({ x, y }))
			{
				const TCHAR* pOccupation = FCString::Strchr(pElementData, TCHAR('o')) + 1;
				bool isOccupied = FCString::Atoi(pOccupation) == 1;

				const TCHAR* pIgnitable = FCString::Strchr(pElementData, TCHAR('i')) + 1;
				bool isIgnitable = FCString::Atoi(pIgnitable) == 1;

				const int32 index = mpPassMapData->IndexFromCoord(FIntPoint(x, y));
				mpPassMapData->mTiles[index].mIsStaticlyOccupied = isOccupied;
				mpPassMapData->mTiles[index].mIsIgnitable = isIgnitable;
			}
		}
	}

	return true;
}

bool FXmlCallback_PassMapData::ProcessAttribute(const TCHAR* pAttributeName,
												const TCHAR* pAttributeValue)
{
	if (IsInitialized() == false && mParsingState == ParsingState::ParsingState_PassMap)
	{
		if (FCString::Stricmp(pAttributeName, sAttr_PassMapWidth) == 0)
		{
			mpPassMapData->mWidth = FCString::Atoi(pAttributeValue);
			if (mpPassMapData->mWidth <= 0)
			{
				return false;
			}
		}
		else if (FCString::Stricmp(pAttributeName, sAttr_PassMapHeight) == 0)
		{
			mpPassMapData->mHeight = FCString::Atoi(pAttributeValue);
			if (mpPassMapData->mHeight <= 0)
			{
				return false;
			}
		}
		else if (FCString::Stricmp(pAttributeName, sAttr_PassMapHeight) == 0)
		{
			mpPassMapData->mTileRadius = FCString::Atof(pAttributeValue);
			if (mpPassMapData->mTileRadius <= 0.0f)
			{
				return false;
			}
		}
		
		if (IsInitialized())
		{
			mpPassMapData->mTiles.SetNum(mpPassMapData->mWidth * mpPassMapData->mHeight);
		}
	}
	return true;
}

bool FXmlCallback_PassMapData::ProcessClose(const TCHAR* pElement)
{
	return true;
}

bool FXmlCallback_PassMapData::ProcessComment(const TCHAR* pComment)
{
	return true;
}


const TCHAR* FXmlCallback_PassMapData::sElem_PassMap = TEXT("passmap");
const TCHAR* FXmlCallback_PassMapData::sElem_Tiles = TEXT("tiles");
const TCHAR* FXmlCallback_PassMapData::sElem_Tile = TEXT("t");
const TCHAR* FXmlCallback_PassMapData::sAttr_TileX = TEXT("x");
const TCHAR* FXmlCallback_PassMapData::sAttr_TileY = TEXT("y");
const TCHAR* FXmlCallback_PassMapData::sAttr_TileIgnition = TEXT("i");
const TCHAR* FXmlCallback_PassMapData::sAttr_TileOccupation = TEXT("o");
const TCHAR* FXmlCallback_PassMapData::sAttr_PassMapWidth = TEXT("width");
const TCHAR* FXmlCallback_PassMapData::sAttr_PassMapHeight = TEXT("height");
const TCHAR* FXmlCallback_PassMapData::sAttr_TileRadius = TEXT("tileradius");

bool FXmlCallback_PassMapData::IsInitialized() const
{
	return mpPassMapData->mWidth > 0 && mpPassMapData->mHeight > 0;
}
