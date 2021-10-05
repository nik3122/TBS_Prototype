// Copyright Epic Games, Inc. All Rights Reserved.

#include "EdModeToolkit_TileMap.h"
#include "Arena/PassMap.h"
#include "Arena/PassMapData.h"
#include "EdMode_TileMap.h"
#include "Engine/Selection.h"
#include "TileMapEditorStyle.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Layout/SScrollBox.h"

#include "EditorModeManager.h"
#include "Brushes/SlateImageBrush.h"
#include "Interfaces/IPluginManager.h"


#define LOCTEXT_NAMESPACE "FEdModeToolkit_TileMap"


FEdModeToolkit_TileMap::FEdModeToolkit_TileMap()
{
}

void FEdModeToolkit_TileMap::Init(const TSharedPtr<IToolkitHost>& InitToolkitHost)
{
    mPanelTileMapParameters = SNew(SBorder)
        .Padding(10)
        .IsEnabled(this, &FEdModeToolkit_TileMap::IsEditingEnabled)
        [
            SNew(SVerticalBox)
            + SVerticalBox::Slot()
                .HAlign(HAlign_Fill)
                .AutoHeight()
                .Padding(0.0f, 10.0f, 0.0f, 0.0f)
                [
                    SNew(SHorizontalBox)
                    + SHorizontalBox::Slot()
                        .AutoWidth()
                        .VAlign(VAlign_Center)
                        [
                            SNew(STextBlock)
                            .Text(LOCTEXT("TileMapSize", "Size: "))
                        ]
                    + SHorizontalBox::Slot()
                        .AutoWidth()
                        .VAlign(VAlign_Center)
                        .Padding(10, 0, 2, 0)
                        [
                            SNew(STextBlock)
                            .Text(LOCTEXT("TileMapSizeY", "Y"))
                        ]
                    + SHorizontalBox::Slot()
                        .AutoWidth()
                        [
                            SNew(SButton)
                                .Text(LOCTEXT("ButtonDecreaseWidth", "-"))
                                .OnClicked(this, &FEdModeToolkit_TileMap::OnButtonClicked_DecreaseWidth)
                        ]
                    + SHorizontalBox::Slot()
                        .AutoWidth()
                        [
                            SNew(SBorder)
                                .VAlign(VAlign_Center)
                            [
                                SNew(STextBlock)
                                    .AutoWrapText(false)
                                    .Text(this, &FEdModeToolkit_TileMap::GetTextTileMapWidth)
                                    .MinDesiredWidth(30)
                            ]
                        ]
                    + SHorizontalBox::Slot()
                        .AutoWidth()
                        [
                            SNew(SButton)
                            .Text(LOCTEXT("ButtonIncreaseWidth", "+"))
                            .OnClicked(this, &FEdModeToolkit_TileMap::OnButtonClicked_IncreaseWidth)
                        ]
                    + SHorizontalBox::Slot()
                        .AutoWidth()
                        .VAlign(VAlign_Center)
                        .Padding(10, 0, 2, 0)
                        [
                            SNew(STextBlock)
                            .Text(LOCTEXT("TileMapSizeX", "X"))
                        ]
                    + SHorizontalBox::Slot()
                        .AutoWidth()
                        [
                            SNew(SButton)
                                .Text(LOCTEXT("ButtonDecreaseHeight", "-"))
                                .OnClicked(this, &FEdModeToolkit_TileMap::OnButtonClicked_DecreaseHeight)
                        ]
                    + SHorizontalBox::Slot()
                        .AutoWidth()
                        [
                            SNew(SBorder)
                                .VAlign(VAlign_Center)
                                [
                                    SNew(STextBlock)
                                        .AutoWrapText(false)
                                        .Text(this, &FEdModeToolkit_TileMap::GetTextTileMapHeight)
                                        .MinDesiredWidth(30)
                                ]
                        ]
                    + SHorizontalBox::Slot()
                        .AutoWidth()
                        [
                            SNew(SButton)
                                .Text(LOCTEXT("ButtonIncreaseHeight", "+"))
                                .OnClicked(this, &FEdModeToolkit_TileMap::OnButtonClicked_IncreaseHeight)
                        ]
                ]
            + SVerticalBox::Slot()
                .HAlign(HAlign_Fill)
                .AutoHeight()
                .Padding(0.0f, 10.0f, 0.0f, 0.0f)
                [
                    SNew(SHorizontalBox)
                    + SHorizontalBox::Slot()
                        .AutoWidth()
                        .VAlign(VAlign_Center)
                        [
                            SNew(STextBlock)
                                .Text(LOCTEXT("TileMapSize", "Size: "))
                        ]
                    + SHorizontalBox::Slot()
                        .AutoWidth()
                        .VAlign(VAlign_Center)
                        .Padding(10, 0, 2, 0)
                        [
                            SNew(STextBlock)
                            .Text(LOCTEXT("TileMapSizeY", "Y"))
                        ]
                    + SHorizontalBox::Slot()
                        .AutoWidth()
                        [
                            SNew(SSpinBox<int32>)
                                .MinDesiredWidth(40)
                                .ToolTipText(LOCTEXT("SpinBoxTileMapWidth_ToolTip", "Width of selected tile map."))
                                .OnValueCommitted(this, &FEdModeToolkit_TileMap::OnSpinBoxCommitted_TileMapWidth)
                                .OnValueChanged(this, &FEdModeToolkit_TileMap::OnSpinBoxChanged_TileMapWidth)
                                .Value_Raw(this, &FEdModeToolkit_TileMap::GetTileMapWidth)
                                .Delta(1)
                                .LinearDeltaSensitivity(600)
                        ]
                    + SHorizontalBox::Slot()
                        .AutoWidth()
                        .VAlign(VAlign_Center)
                        .Padding(10, 0, 2, 0)
                        [
                            SNew(STextBlock)
                            .Text(LOCTEXT("TileMapSizeX", "X"))
                        ]
                    + SHorizontalBox::Slot()
                        .AutoWidth()
                        [
                            SNew(SSpinBox<int32>)
                                .MinDesiredWidth(40)
                                .ToolTipText(LOCTEXT("SpinBoxTileMapHeight_ToolTip", "Height of selected tile map."))
                                .OnValueCommitted(this, &FEdModeToolkit_TileMap::OnSpinBoxCommitted_TileMapHeight)
                                .OnValueChanged(this, &FEdModeToolkit_TileMap::OnSpinBoxChanged_TileMapHeight)
                                .Value_Raw(this, &FEdModeToolkit_TileMap::GetTileMapHeight)
                                .Delta(1)
                                .LinearDeltaSensitivity(600)
                        ]
                ]
        ];

        const float edittingModesPadding = 0.0f;

        mPanelTileMapEditing = SNew(SBorder)
            .VAlign(VAlign_Top)
            .Padding(10)
            .IsEnabled(this, &FEdModeToolkit_TileMap::IsEditingEnabled)
            [
                SNew(SScrollBox)
                    .AllowOverscroll(EAllowOverscroll::No)
                + SScrollBox::Slot()
                    [
                        SNew(SVerticalBox)
                        + SVerticalBox::Slot()
                            .HAlign(HAlign_Fill)
                            [
                                SNew(SHorizontalBox)
                                +SHorizontalBox::Slot()
                                    [
                                        SNew(SCheckBox)
                                            .OnCheckStateChanged(this, &FEdModeToolkit_TileMap::OnCheckBoxStateChanged_PickTile)
                                            .IsChecked(this, &FEdModeToolkit_TileMap::GetCheckBoxState_PickTile)
                                            .Style(&FTileMapEditorStyle::GetCheckBoxStyle("TileMapEditor.EditingMode.PickTile"))
                                            .ToolTipText(LOCTEXT("EditingModePickTile", "Pick Tile"))
                                            [
                                                SNew(STextBlock)
                                                    .Text(LOCTEXT("EditingModePickTile", "Pick Tile"))
                                            ]
                                    ]
                                +SHorizontalBox::Slot()
                                    .Padding(10, 0, 0, 0)
                                    .VAlign(VAlign_Center)
                                    [
                                        SNew(STextBlock)
                                            .Text(this, &FEdModeToolkit_TileMap::GetText_TileUnderCursor)
                                    ]
                            ]
                        + SVerticalBox::Slot()
                            .HAlign(HAlign_Fill)
                            [
                                SNew(SHorizontalBox)
                                +SHorizontalBox::Slot()
                                [
                                    SNew(SCheckBox)
                                        .OnCheckStateChanged(this, &FEdModeToolkit_TileMap::OnCheckBoxStateChanged_ResetTile)
                                        .IsChecked(this, &FEdModeToolkit_TileMap::GetCheckBoxState_ResetTile)
                                        .Style(&FTileMapEditorStyle::GetCheckBoxStyle("TileMapEditor.EditingMode.ResetTile"))
                                        .ToolTipText(LOCTEXT("EditingModeResetTile", "Reset Tile"))
                                    [
                                        SNew(STextBlock)
                                            .Text(LOCTEXT("EditingModeResetTile", "Reset Tile"))
                                    ]
                                ]
                            ]
                        + SVerticalBox::Slot()
                            .HAlign(HAlign_Fill)
                            .Padding(0.f, edittingModesPadding, 0.f, 0.f)
                            [
                                SNew(SHorizontalBox)
                                +SHorizontalBox::Slot()
                                    [
                                        SNew(SCheckBox)
                                            .OnCheckStateChanged(this, &FEdModeToolkit_TileMap::OnCheckBoxStateChanged_DrawLowObstacle)
                                            .IsChecked(this, &FEdModeToolkit_TileMap::GetCheckBoxState_DrawLowObstacle)
                                            .Style(&FTileMapEditorStyle::GetCheckBoxStyle("TileMapEditor.EditingMode.DrawLowObstacle"))
                                            .ToolTipText(LOCTEXT("EditingModeDrawLowObstacle", "Draw Low Obstacle"))
                                            [
                                                SNew(STextBlock)
                                                .Text(LOCTEXT("EditingModeDrawLowObstacle", "Draw Low Obstacle"))
                                            ]
                                    ]
                            ]
                        + SVerticalBox::Slot()
                            .HAlign(HAlign_Fill)
                            .Padding(0.f, edittingModesPadding, 0.f, 0.f)
                            [
                                SNew(SHorizontalBox)
                                +SHorizontalBox::Slot()
                                    [
                                        SNew(SCheckBox)
                                            .OnCheckStateChanged(this, &FEdModeToolkit_TileMap::OnCheckBoxStateChanged_UndrawLowObstacle)
                                            .IsChecked(this, &FEdModeToolkit_TileMap::GetCheckBoxState_UndrawLowObstacle)
                                            .Style(&FTileMapEditorStyle::GetCheckBoxStyle("TileMapEditor.EditingMode.UndrawLowObstacle"))
                                            .ToolTipText(LOCTEXT("EditingModeUndrawLowObstacle", "Undraw Low Obstacle"))
                                            [
                                                SNew(STextBlock)
                                                .Text(LOCTEXT("EditingModeUndrawLowObstacle", "Undraw Low Obstacle"))
                                            ]
                                    ]
                            ]
                        + SVerticalBox::Slot()
                            .HAlign(HAlign_Fill)
                            .Padding(0.f, edittingModesPadding, 0.f, 0.f)
                            [
                                SNew(SHorizontalBox)
                                +SHorizontalBox::Slot()
                                    [
                                        SNew(SCheckBox)
                                            .OnCheckStateChanged(this, &FEdModeToolkit_TileMap::OnCheckBoxStateChanged_DrawHighObstacle)
                                            .IsChecked(this, &FEdModeToolkit_TileMap::GetCheckBoxState_DrawHighObstacle)
                                            .Style(&FTileMapEditorStyle::GetCheckBoxStyle("TileMapEditor.EditingMode.DrawHighObstacle"))
                                            .ToolTipText(LOCTEXT("EditingModeDrawHighObstacle", "Draw High Obstacle"))
                                            [
                                                SNew(STextBlock)
                                                .Text(LOCTEXT("EditingModeDrawHighObstacle", "Draw High Obstacle"))
                                            ]
                                    ]
                            ]
                        + SVerticalBox::Slot()
                            .HAlign(HAlign_Fill)
                            .Padding(0.f, edittingModesPadding, 0.f, 0.f)
                            [
                                SNew(SHorizontalBox)
                                +SHorizontalBox::Slot()
                                    [
                                        SNew(SCheckBox)
                                            .OnCheckStateChanged(this, &FEdModeToolkit_TileMap::OnCheckBoxStateChanged_UndrawHighObstacle)
                                            .IsChecked(this, &FEdModeToolkit_TileMap::GetCheckBoxState_UndrawHighObstacle)
                                            .Style(&FTileMapEditorStyle::GetCheckBoxStyle("TileMapEditor.EditingMode.UndrawHighObstacle"))
                                            .ToolTipText(LOCTEXT("EditingModeUndrawHighObstacle", "Undraw High Obstacle"))
                                            [
                                                SNew(STextBlock)
                                                .Text(LOCTEXT("EditingModeUndrawHighObstacle", "Undraw High Obstacle"))
                                            ]
                                    ]
                            ]
                        + SVerticalBox::Slot()
                            .HAlign(HAlign_Fill)
                            .Padding(0.f, edittingModesPadding, 0.f, 0.f)
                            [
                                SNew(SHorizontalBox)
                                +SHorizontalBox::Slot()
                                    [
                                        SNew(SCheckBox)
                                            .OnCheckStateChanged(this, &FEdModeToolkit_TileMap::OnCheckBoxStateChanged_DrawFire)
                                            .IsChecked(this, &FEdModeToolkit_TileMap::GetCheckBoxState_DrawFire)
                                            .Style(&FTileMapEditorStyle::GetCheckBoxStyle("TileMapEditor.EditingMode.DrawFire"))
                                            .ToolTipText(LOCTEXT("EditingModeDrawFire", "Draw Fire"))
                                            [
                                                SNew(STextBlock)
                                                .Text(LOCTEXT("EditingModeDrawFire", "Draw Fire"))
                                            ]
                                    ]
                            ]
                        + SVerticalBox::Slot()
                            .HAlign(HAlign_Fill)
                            .Padding(0.f, edittingModesPadding, 0.f, 0.f)
                            [
                                SNew(SHorizontalBox)
                                +SHorizontalBox::Slot()
                                    [
                                        SNew(SCheckBox)
                                            .OnCheckStateChanged(this, &FEdModeToolkit_TileMap::OnCheckBoxStateChanged_UndrawFire)
                                            .IsChecked(this, &FEdModeToolkit_TileMap::GetCheckBoxState_UndrawFire)
                                            .Style(&FTileMapEditorStyle::GetCheckBoxStyle("TileMapEditor.EditingMode.UndrawFire"))
                                            .ToolTipText(LOCTEXT("EditingModeUndrawFire", "Undraw Fire"))
                                            [
                                                SNew(STextBlock)
                                                .Text(LOCTEXT("EditingModeUndrawFire", "Undraw Fire"))
                                            ]
                                    ]

                            ]
                    ]
            ];

    SAssignNew(ToolkitWidget, SBorder)
        .HAlign(HAlign_Fill)
        .Padding(15)
        [
            SNew(SVerticalBox)
            + SVerticalBox::Slot()
                .HAlign(HAlign_Fill)
                .AutoHeight()
                [
                    SNew(STextBlock)
                        .AutoWrapText(true)
                        .Text(this, &FEdModeToolkit_TileMap::GetTextTileMapParameters)
                ]
            + SVerticalBox::Slot()
                .HAlign(HAlign_Left)
                .AutoHeight()
                .Padding(0.f, 10.f, 0.f, 0.f)
                [
                    SNew(SButton)
                        .Text(LOCTEXT("ButtonCreateTileMapData", "+TileMapData"))
                        .ToolTipText(LOCTEXT("ButtonCreateTileMapDataToolTip", "Create TileMapData asset and associate it with the selected tile map."))
                        .OnClicked(this, &FEdModeToolkit_TileMap::OnButtonClicked_CreateTileMapData)
                        .IsEnabled(this, &FEdModeToolkit_TileMap::IsEnabled_CreateTileMapData)
                ]
            + SVerticalBox::Slot()
                .HAlign(HAlign_Fill)
                .AutoHeight()
                .Padding(0.f, 10.f, 0.f, 0.f)
                [
                    mPanelTileMapParameters.ToSharedRef()
                ]
            + SVerticalBox::Slot()
                .HAlign(HAlign_Fill)
                .VAlign(VAlign_Fill)
                .Padding(0.f, 10.f, 0.f, 0.f)
                [
                    mPanelTileMapEditing.ToSharedRef()
                ]
        ];
        
    FModeToolkit::Init(InitToolkitHost);
}

void FEdModeToolkit_TileMap::BuildToolPalette(FName palette,
                                              class FToolBarBuilder& toolbarBuilder)
{
    FModeToolkit::BuildToolPalette(palette, toolbarBuilder);
}

FName FEdModeToolkit_TileMap::GetToolkitFName() const
{
    return FName("TileMapEditorEdMode");
}

FText FEdModeToolkit_TileMap::GetBaseToolkitName() const
{
    return NSLOCTEXT("TileMapEditorEdModeToolkit", "DisplayName", "TileMapEditorEdMode Tool");
}

FEdMode_TileMap* FEdModeToolkit_TileMap::GetEditorMode() const
{
    FEdMode* edMode = GLevelEditorModeTools().GetActiveMode(FEdMode_TileMap::EM_TileMapEditorEdModeId);
    return static_cast<FEdMode_TileMap*>(edMode);
}

void FEdModeToolkit_TileMap::RefreshDetailPanel()
{
    FEdMode_TileMap* edMode = GetEditorMode();
    APassMap* tileMap = edMode->GetTileMap();
    if (tileMap == nullptr)
    {
        mPanelTileMapParameters->SetEnabled(false);
    }
    else
    {
        mPanelTileMapParameters->SetEnabled(true);
    }
}

bool FEdModeToolkit_TileMap::IsEditingEnabled() const
{
    FEdMode_TileMap* edMode = GetEditorMode();
    return edMode->GetCurrentEditingMode() != ETileMapEditingMode::Invalid;
}

void FEdModeToolkit_TileMap::SetTileMapWidth(int32 newTileMapWidth)
{
    newTileMapWidth = FMath::Clamp(newTileMapWidth, 1, 666);
    FEdMode_TileMap* edMode = GetEditorMode();
    APassMap* tileMap = edMode->GetTileMap();
    if (tileMap != nullptr)
    {
        int32 height = tileMap->GetHeight();
        edMode->ResizeTileMap(newTileMapWidth, height);
    }
}

void FEdModeToolkit_TileMap::SetTileMapHeight(int32 newTileMapHeight)
{
    newTileMapHeight = FMath::Clamp(newTileMapHeight, 1, 666);
    FEdMode_TileMap* edMode = GetEditorMode();
    APassMap* tileMap = edMode->GetTileMap();
    if (tileMap != nullptr)
    {
        int32 width = tileMap->GetWidth();
        edMode->ResizeTileMap(width, newTileMapHeight);
    }
}

int32 FEdModeToolkit_TileMap::GetTileMapWidth() const
{
    FEdMode_TileMap* edMode = GetEditorMode();
    APassMap* tileMap = edMode->GetTileMap();
    if (tileMap != nullptr)
    {
        UPassMapData* passMapData = tileMap->GetPassMapData();
        return passMapData != nullptr ? passMapData->mWidth : tileMap->GetWidth();
    }
    return 0;
}

int32 FEdModeToolkit_TileMap::GetTileMapHeight() const
{
    FEdMode_TileMap* edMode = GetEditorMode();
    APassMap* tileMap = edMode->GetTileMap();
    if (tileMap != nullptr)
    {
        UPassMapData* passMapData = tileMap->GetPassMapData();
        return passMapData != nullptr ? passMapData->mHeight : tileMap->GetHeight();
    }
    return 0;
}

FText FEdModeToolkit_TileMap::GetTextTileMapParameters() const
{
    FEdMode_TileMap* edMode = GetEditorMode();
    APassMap* tileMap = edMode->GetTileMap();
    if (tileMap != nullptr)
    {
        UPassMapData* passMapData = tileMap->GetPassMapData();
        return passMapData == nullptr ? LOCTEXT("NoTileMapData", "Tile map has no associated UPassMapData. Create new one.") :
            LOCTEXT("SelectedTileMapHasData", "Editing selected tile map data.");
    }
    return LOCTEXT("NoTileMapSelected", "Please select APassMap actor for editing.");
}

FText FEdModeToolkit_TileMap::GetTextTileMapWidth() const
{
    return FText::FromString(FString::FromInt(GetTileMapWidth()));
}

FText FEdModeToolkit_TileMap::GetTextTileMapHeight() const
{
    return FText::FromString(FString::FromInt(GetTileMapHeight()));
}

bool FEdModeToolkit_TileMap::IsEnabled_CreateTileMapData() const
{
    FEdMode_TileMap* edMode = GetEditorMode();
    APassMap* tileMap = edMode->GetTileMap();
    return !(tileMap == nullptr || tileMap->GetPassMapData() != nullptr);
}

FReply FEdModeToolkit_TileMap::OnButtonClicked_CreateTileMapData()
{
    FEdMode_TileMap* edMode = GetEditorMode();
    edMode->CreateTileMapDataForSelectedTileMap();
    return FReply::Handled();
}

void FEdModeToolkit_TileMap::OnSpinBoxChanged_TileMapWidth(int32 newWidth)
{
    SetTileMapWidth(newWidth);
}

void FEdModeToolkit_TileMap::OnSpinBoxCommitted_TileMapWidth(int32 newWidth,
                                                             ETextCommit::Type)
{
    SetTileMapWidth(newWidth);
}

void FEdModeToolkit_TileMap::OnSpinBoxChanged_TileMapHeight(int32 newHeight)
{
    SetTileMapHeight(newHeight);
}

void FEdModeToolkit_TileMap::OnSpinBoxCommitted_TileMapHeight(int32 newHeight,
                                                              ETextCommit::Type)
{
    SetTileMapHeight(newHeight);
}

FReply FEdModeToolkit_TileMap::OnButtonClicked_IncreaseWidth()
{
    SetTileMapWidth(GetTileMapWidth() + 1);
    return FReply::Handled();
}

FReply FEdModeToolkit_TileMap::OnButtonClicked_DecreaseWidth()
{
    SetTileMapWidth(GetTileMapWidth() - 1);
    return FReply::Handled();
}

FReply FEdModeToolkit_TileMap::OnButtonClicked_IncreaseHeight()
{
    SetTileMapHeight(GetTileMapHeight() + 1);
    return FReply::Handled();
}

FReply FEdModeToolkit_TileMap::OnButtonClicked_DecreaseHeight()
{
    SetTileMapHeight(GetTileMapWidth() + 1);
    return FReply::Handled();
}

void FEdModeToolkit_TileMap::OnCheckBoxStateChanged_PickTile(ECheckBoxState state)
{
    if (state == ECheckBoxState::Checked)
    {
        GetEditorMode()->SetCurrentEditingMode(ETileMapEditingMode::PickTile);
    }
    else if (state == ECheckBoxState::Unchecked &&
             GetEditorMode()->GetCurrentEditingMode() == ETileMapEditingMode::PickTile)
    {
        GetEditorMode()->SetCurrentEditingMode(ETileMapEditingMode::None);
    }
}

ECheckBoxState FEdModeToolkit_TileMap::GetCheckBoxState_PickTile() const
{
    return GetEditorMode()->GetCurrentEditingMode() == ETileMapEditingMode::PickTile ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
}

void FEdModeToolkit_TileMap::OnCheckBoxStateChanged_ResetTile(ECheckBoxState state)
{
    if (state == ECheckBoxState::Checked)
    {
        GetEditorMode()->SetCurrentEditingMode(ETileMapEditingMode::ResetTile);
    }
    else if (state == ECheckBoxState::Unchecked &&
             GetEditorMode()->GetCurrentEditingMode() == ETileMapEditingMode::ResetTile)
    {
        GetEditorMode()->SetCurrentEditingMode(ETileMapEditingMode::None);
    }
}

ECheckBoxState FEdModeToolkit_TileMap::GetCheckBoxState_ResetTile() const
{
    return GetEditorMode()->GetCurrentEditingMode() == ETileMapEditingMode::ResetTile ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
}

void FEdModeToolkit_TileMap::OnCheckBoxStateChanged_DrawLowObstacle(ECheckBoxState state)
{
    if (state == ECheckBoxState::Checked)
    {
        GetEditorMode()->SetCurrentEditingMode(ETileMapEditingMode::DrawLowObstacle);
    }
    else if (state == ECheckBoxState::Unchecked &&
             GetEditorMode()->GetCurrentEditingMode() == ETileMapEditingMode::DrawLowObstacle)
    {
        GetEditorMode()->SetCurrentEditingMode(ETileMapEditingMode::None);
    }
}

ECheckBoxState FEdModeToolkit_TileMap::GetCheckBoxState_DrawLowObstacle() const
{
    return GetEditorMode()->GetCurrentEditingMode() == ETileMapEditingMode::DrawLowObstacle ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
}

void FEdModeToolkit_TileMap::OnCheckBoxStateChanged_UndrawLowObstacle(ECheckBoxState state)
{
    if (state == ECheckBoxState::Checked)
    {
        GetEditorMode()->SetCurrentEditingMode(ETileMapEditingMode::UndrawLowObstacle);
    }
    else if (state == ECheckBoxState::Unchecked &&
             GetEditorMode()->GetCurrentEditingMode() == ETileMapEditingMode::UndrawLowObstacle)
    {
        GetEditorMode()->SetCurrentEditingMode(ETileMapEditingMode::None);
    }
}

ECheckBoxState FEdModeToolkit_TileMap::GetCheckBoxState_UndrawLowObstacle() const
{
    return GetEditorMode()->GetCurrentEditingMode() == ETileMapEditingMode::UndrawLowObstacle ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
}

void FEdModeToolkit_TileMap::OnCheckBoxStateChanged_DrawHighObstacle(ECheckBoxState state)
{
    if (state == ECheckBoxState::Checked)
    {
        GetEditorMode()->SetCurrentEditingMode(ETileMapEditingMode::DrawHighObstacle);
    }
    else if (state == ECheckBoxState::Unchecked &&
             GetEditorMode()->GetCurrentEditingMode() == ETileMapEditingMode::DrawHighObstacle)
    {
        GetEditorMode()->SetCurrentEditingMode(ETileMapEditingMode::None);
    }
}

ECheckBoxState FEdModeToolkit_TileMap::GetCheckBoxState_DrawHighObstacle() const
{
    return GetEditorMode()->GetCurrentEditingMode() == ETileMapEditingMode::DrawHighObstacle ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
}

void FEdModeToolkit_TileMap::OnCheckBoxStateChanged_UndrawHighObstacle(ECheckBoxState state)
{
    if (state == ECheckBoxState::Checked)
    {
        GetEditorMode()->SetCurrentEditingMode(ETileMapEditingMode::UndrawHighObstacle);
    }
    else if (state == ECheckBoxState::Unchecked &&
             GetEditorMode()->GetCurrentEditingMode() == ETileMapEditingMode::UndrawHighObstacle)
    {
        GetEditorMode()->SetCurrentEditingMode(ETileMapEditingMode::None);
    }
}

ECheckBoxState FEdModeToolkit_TileMap::GetCheckBoxState_UndrawHighObstacle() const
{
    return GetEditorMode()->GetCurrentEditingMode() == ETileMapEditingMode::UndrawHighObstacle ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
}

void FEdModeToolkit_TileMap::OnCheckBoxStateChanged_DrawFire(ECheckBoxState state)
{
    if (state == ECheckBoxState::Checked)
    {
        GetEditorMode()->SetCurrentEditingMode(ETileMapEditingMode::DrawFire);
    }
    else if (state == ECheckBoxState::Unchecked &&
             GetEditorMode()->GetCurrentEditingMode() == ETileMapEditingMode::DrawFire)
    {
        GetEditorMode()->SetCurrentEditingMode(ETileMapEditingMode::None);
    }
}

ECheckBoxState FEdModeToolkit_TileMap::GetCheckBoxState_DrawFire() const
{
    return GetEditorMode()->GetCurrentEditingMode() == ETileMapEditingMode::DrawFire ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
}

void FEdModeToolkit_TileMap::OnCheckBoxStateChanged_UndrawFire(ECheckBoxState state)
{
    if (state == ECheckBoxState::Checked)
    {
        GetEditorMode()->SetCurrentEditingMode(ETileMapEditingMode::UndrawFire);
    }
    else if (state == ECheckBoxState::Unchecked &&
             GetEditorMode()->GetCurrentEditingMode() == ETileMapEditingMode::UndrawFire)
    {
        GetEditorMode()->SetCurrentEditingMode(ETileMapEditingMode::None);
    }
}

ECheckBoxState FEdModeToolkit_TileMap::GetCheckBoxState_UndrawFire() const
{
    return GetEditorMode()->GetCurrentEditingMode() == ETileMapEditingMode::UndrawFire ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
}

FText FEdModeToolkit_TileMap::GetText_TileUnderCursor() const
{
    FIntPoint tile = GetEditorMode()->GetTileUnderCursor();
    FString tileString = FString::Format(TEXT("({0};{1})"), { 
                                            tile.X, tile.Y 
                                         });
    return FText::FromString(tileString);
}

#undef LOCTEXT_NAMESPACE
