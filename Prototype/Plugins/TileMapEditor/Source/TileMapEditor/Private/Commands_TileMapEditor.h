#pragma once


#include "CoreMinimal.h"
#include "Framework/Commands/Commands.h"


class FCommands_TileMapEditor
    : public TCommands<FCommands_TileMapEditor>
{
public:

    FCommands_TileMapEditor();

    void RegisterCommands() override;

public:
};