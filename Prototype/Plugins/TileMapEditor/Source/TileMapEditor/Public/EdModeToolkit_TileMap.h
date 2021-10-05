#pragma once


#include "CoreMinimal.h"
#include "EdMode_TileMap.h"
#include "Toolkits/BaseToolkit.h"
#include "Widgets/Input/SCheckBox.h"
#include "Widgets/Input/SSpinBox.h"


class STextBlock;
class FEdMode_TileMap;


class FEdModeToolkit_TileMap 
    : public FModeToolkit
{
public:

    FEdModeToolkit_TileMap();

    /** FModeToolkit interface */
    void Init(const TSharedPtr<IToolkitHost>& InitToolkitHost) override;

    void BuildToolPalette(FName palette,
                          class FToolBarBuilder& toolbarBuilder) override;

    /** IToolkit interface */
    FName GetToolkitFName() const override;
    FText GetBaseToolkitName() const override;
    FEdMode_TileMap* GetEditorMode() const override;
    TSharedPtr<class SWidget> GetInlineContent() const override { return ToolkitWidget; }

    void RefreshDetailPanel();

private:

    TSharedPtr<SWidget> ToolkitWidget;

    TSharedPtr<SBorder> mPanelTileMapParameters;

    TSharedPtr<SBorder> mPanelTileMapEditing;

private:

    bool IsEditingEnabled() const;

    void SetTileMapWidth(int32 newTileMapWidth);

    void SetTileMapHeight(int32 newTileMapHeight);

    int32 GetTileMapWidth() const;

    int32 GetTileMapHeight() const;

    FText GetTextTileMapParameters() const;

    FText GetTextTileMapWidth() const;

    FText GetTextTileMapHeight() const;

    bool IsEnabled_CreateTileMapData() const;

    FReply OnButtonClicked_CreateTileMapData();

    void OnSpinBoxChanged_TileMapWidth(int32 newWidth);

    void OnSpinBoxCommitted_TileMapWidth(int32 newWidth,
                                         ETextCommit::Type);

    void OnSpinBoxChanged_TileMapHeight(int32 newHeight);

    void OnSpinBoxCommitted_TileMapHeight(int32 newHeight,
                                          ETextCommit::Type);

    FReply OnButtonClicked_IncreaseWidth();

    FReply OnButtonClicked_DecreaseWidth();

    FReply OnButtonClicked_IncreaseHeight();

    FReply OnButtonClicked_DecreaseHeight();

    void OnCheckBoxStateChanged_PickTile(ECheckBoxState state);

    ECheckBoxState GetCheckBoxState_PickTile() const;

    void OnCheckBoxStateChanged_ResetTile(ECheckBoxState state);

    ECheckBoxState GetCheckBoxState_ResetTile() const;

    void OnCheckBoxStateChanged_DrawLowObstacle(ECheckBoxState state);

    ECheckBoxState GetCheckBoxState_DrawLowObstacle() const;

    void OnCheckBoxStateChanged_UndrawLowObstacle(ECheckBoxState state);

    ECheckBoxState GetCheckBoxState_UndrawLowObstacle() const;

    void OnCheckBoxStateChanged_DrawHighObstacle(ECheckBoxState state);

    ECheckBoxState GetCheckBoxState_DrawHighObstacle() const;

    void OnCheckBoxStateChanged_UndrawHighObstacle(ECheckBoxState state);

    ECheckBoxState GetCheckBoxState_UndrawHighObstacle() const;

    void OnCheckBoxStateChanged_DrawFire(ECheckBoxState state);

    ECheckBoxState GetCheckBoxState_DrawFire() const;

    void OnCheckBoxStateChanged_UndrawFire(ECheckBoxState state);

    ECheckBoxState GetCheckBoxState_UndrawFire() const;

    FText GetText_TileUnderCursor() const;
};
