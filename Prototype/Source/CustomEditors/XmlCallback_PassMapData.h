#pragma once


#include "../XmlParser/Public/FastXml.h"
#include "Arena/PassMapData.h"


class FXmlCallback_PassMapData:
	public IFastXmlCallback
{
public:
	FXmlCallback_PassMapData();
	~FXmlCallback_PassMapData() = default;

	bool LoadPassMapData(UPassMapData* pPassMapData,
						 FString& xmlData,
						 bool isFilePath,
						 FFeedbackContext* pFeedbackContext);

	//	IFastXmlCallback::
	virtual bool ProcessXmlDeclaration( const TCHAR* pElementData, int32 XmlFileLineNumber ) final;
	virtual bool ProcessElement( const TCHAR* pElementName, const TCHAR* pElementData, int32 XmlFileLineNumber ) final;
	virtual bool ProcessAttribute( const TCHAR* pAttributeName, const TCHAR* pAttributeValue ) final;
	virtual bool ProcessClose( const TCHAR* pElement ) final;
	virtual bool ProcessComment(const TCHAR* pComment) final;

private:
	static const TCHAR* sElem_PassMap;
	static const TCHAR* sElem_Tiles;
	static const TCHAR* sElem_Tile;
	static const TCHAR* sAttr_TileX;
	static const TCHAR* sAttr_TileY;
	static const TCHAR* sAttr_TileIgnition;
	static const TCHAR* sAttr_TileOccupation;
	static const TCHAR* sAttr_PassMapWidth;
	static const TCHAR* sAttr_PassMapHeight;
	static const TCHAR* sAttr_TileRadius;

	enum class ParsingState
	{
		ParsingState_Uninitialized,
		ParsingState_PassMap,
		ParsingState_Tiles,
		ParsingState_Tile,
	};

private:
	ParsingState mParsingState;
	UPassMapData* mpPassMapData;
	int32 mX;
	int32 mY;


private:
	bool IsInitialized() const;
};