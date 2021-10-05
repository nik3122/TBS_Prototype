
#include "Commands_TileMapEditor.h"


#define LOCTEXT_NAMESPACE "TileMapEditorCommands"


FCommands_TileMapEditor::FCommands_TileMapEditor()
    : TCommands<FCommands_TileMapEditor>(TEXT("TileMap"),
                                         LOCTEXT("TileMapEditor", "Tile Map Editor"),
                                         NAME_None,
                                         NAME_None)
{

}

void FCommands_TileMapEditor::RegisterCommands()
{

}

#undef LOCTEXT_NAMESPACE