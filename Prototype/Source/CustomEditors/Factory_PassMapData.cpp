// Fill out your copyright notice in the Description page of Project Settings.


#include "Factory_PassMapData.h"
#include "Arena/PassMapData.h"
#include "AssetTypeCategories.h"
#include "XmlCallback_PassMapData.h"

UFactory_PassMapData::UFactory_PassMapData(const class FObjectInitializer& initializer):
	Super(initializer)
{
	Formats.Add(TEXT("psmp;PassMap data XML file."));

	bCreateNew = false;
	bEditorImport = true;
	bText = true;

	SupportedClass = UPassMapData::StaticClass();
}

uint32 UFactory_PassMapData::GetMenuCategories() const
{
	return EAssetTypeCategories::Blueprint;
}

bool UFactory_PassMapData::FactoryCanImport(const FString& filename)
{
	return true;
}

UObject* UFactory_PassMapData::FactoryCreateText(UClass* inClass,
												 UObject* inParent,
												 FName inName,
												 EObjectFlags flags,
												 UObject* context,
												 const TCHAR* type,
												 const TCHAR*& buffer,
												 const TCHAR* bufferEnd,
												 FFeedbackContext* feedback)
{
	UPassMapData* pPassMapData = NewObject<UPassMapData>(inParent, inName, flags);

	FString xmlData(bufferEnd - buffer, buffer);

	FXmlCallback_PassMapData xmlParser;
	xmlParser.LoadPassMapData(pPassMapData, xmlData, false, feedback);

	return pPassMapData;
}

UObject* UFactory_PassMapData::FactoryCreateNew(UClass* inClass,
												UObject* inParent,
												FName inName,
												EObjectFlags flags,
												UObject* context,
												FFeedbackContext* warn,
												FName callingContext)
{
	UPassMapData* pPassMapData = NewObject<UPassMapData>(inParent, inName, flags);

	return nullptr;
}
