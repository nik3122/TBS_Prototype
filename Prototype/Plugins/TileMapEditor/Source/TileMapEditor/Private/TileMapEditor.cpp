// Copyright Epic Games, Inc. All Rights Reserved.

#include "TileMapEditor.h"
#include "EdMode_TileMap.h"
#include "Commands_TileMapEditor.h"
#include "TileMapEditorStyle.h"


#define LOCTEXT_NAMESPACE "FTileMapEditorModule"

void FTileMapEditorModule::StartupModule()
{
    FTileMapEditorStyle::Initialize();

    // This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
    FEditorModeRegistry::Get().RegisterMode<FEdMode_TileMap>(FEdMode_TileMap::EM_TileMapEditorEdModeId,
                                                             LOCTEXT("TileMapEditorEdModeName", "TileMapEditorEdMode"),
                                                             FSlateIcon("TileMapEditorStyle", "TileMapEditorMode", "TileMapEditorMode.Small"),
                                                             true);

    //FCommands_TileMapEditor::Register();

}

void FTileMapEditorModule::ShutdownModule()
{
    // This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
    // we call this function before unloading the module.
    FEditorModeRegistry::Get().UnregisterMode(FEdMode_TileMap::EM_TileMapEditorEdModeId);

    //FCommands_TileMapEditor::Unregister();

    FTileMapEditorStyle::Shutdown();
}

#undef LOCTEXT_NAMESPACE
    

IMPLEMENT_MODULE(FTileMapEditorModule, TileMapEditor)