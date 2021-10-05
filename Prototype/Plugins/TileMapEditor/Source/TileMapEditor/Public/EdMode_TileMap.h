// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "EdMode.h"


class APassMap;
class UPassMapData;


enum class ETileMapEditingMode
{
    Invalid = -1,               //  Tile map is either not selected or it's tile map data is not set.
    None = 0,                   //  No "tool" is selected, but the tile map is properly initialized.
    PickTile,
    ResetTile,
    DrawLowObstacle,
    UndrawLowObstacle,
    DrawHighObstacle,
    UndrawHighObstacle,
    DrawFire,
    UndrawFire,
    MAX,
};


constexpr const TCHAR* const ToString(ETileMapEditingMode editingMode)
{
    switch (editingMode)
    {
    case ETileMapEditingMode::None:
        return TEXT("None");

    case ETileMapEditingMode::PickTile:
        return TEXT("PickTile");

    case ETileMapEditingMode::ResetTile:
        return TEXT("ResetTile");

    case ETileMapEditingMode::DrawLowObstacle:
        return TEXT("DrawLowObstacle");

    case ETileMapEditingMode::UndrawLowObstacle:
        return TEXT("UndrawLowObstacle");

    case ETileMapEditingMode::DrawHighObstacle:
        return TEXT("DrawHighObstacle");

    case ETileMapEditingMode::UndrawHighObstacle:
        return TEXT("UndrawHighObstacle");

    case ETileMapEditingMode::DrawFire:
        return TEXT("DrawFire");

    case ETileMapEditingMode::UndrawFire:
        return TEXT("UndrawFire");
    }
    return TEXT("INVALID");
}


class FEdMode_TileMap
    : public FEdMode
{
public:

    const static FEditorModeID EM_TileMapEditorEdModeId;

public:
    FEdMode_TileMap();
    virtual ~FEdMode_TileMap();

    // FEdMode interface

    void Enter() override;

    void Exit() override;

    void Tick(FEditorViewportClient* viewportClient,
              float deltaTime) override;

    //void Render(const FSceneView* View, FViewport* Viewport, FPrimitiveDrawInterface* PDI) override;
    //void ActorSelectionChangeNotify() override;

    bool UsesToolkits() const override;

    bool MouseMove(FEditorViewportClient* viewportClient,
                   FViewport* viewport,
                   int32 x, int32 y) override;

    bool HandleClick(FEditorViewportClient* viewportClient,
                     HHitProxy* hitProxy,
                     const FViewportClick& click) override;

    bool InputKey(FEditorViewportClient* viewportClient,
                  FViewport* viewport,
                  FKey key,
                  EInputEvent event) override;

    bool DisallowMouseDeltaTracking() const override;

    bool Select(AActor* actor,
                bool isSelected) override;

    bool IsSelectionAllowed(AActor* actor,
                            bool isInSelection) const override;

    bool AllowsViewportDragTool() const override;

    bool UsesTransformWidget() const override;

    bool ShouldDrawWidget() const override;

    // End of FEdMode interface

    APassMap* GetTileMap() const;

    void RefreshDetailPanel();

    bool SetCurrentEditingMode(ETileMapEditingMode editingMode);

    ETileMapEditingMode GetCurrentEditingMode() const;

    FIntPoint GetTileUnderCursor() const;

    UPassMapData* CreateTileMapDataForSelectedTileMap();

    void ResizeTileMap(int32 newWidth, int32 newHeight);

private:

    TWeakObjectPtr<APassMap> mPassMap;

    ETileMapEditingMode mCurrentEditingMode = ETileMapEditingMode::Invalid;

    FIntPoint mTileUnderCursor;

    bool mIsDrawing = false;

private:

    void SetPassMap(APassMap* newPassMap);

    void StartDrawing();

    void StopDrawing();

    bool IsDrawing();

    void HandleDrawing(FEditorViewportClient* viewportClient);

};
