

#include "TileMapEditorStyle.h"
#include "Interfaces/IPluginManager.h"
#include "Styling/SlateStyleRegistry.h"
#include "Styling/SlateTypes.h"


#define IMAGE_BRUSH(RelativePath, ...) FSlateImageBrush(mStyleSet->RootToContentDir(RelativePath, TEXT( ".png" )), __VA_ARGS__)


TSharedPtr<FSlateStyleSet> FTileMapEditorStyle::mStyleSet = nullptr;


void FTileMapEditorStyle::Initialize()
{
    if(mStyleSet.IsValid())
    {
        return;
    }

    TSharedPtr<IPlugin> plugin = IPluginManager::Get().FindPlugin(TEXT("TileMapEditor"));
    if (plugin.IsValid() == false)
    {
        return;
    }

    mStyleSet = MakeShared<FSlateStyleSet>(GetStyleSetName());

    
    FString pluginPath = plugin->GetBaseDir();
    FString pluginResourcesPath = pluginPath / TEXT("Resources");
    FString pluginContentPath = plugin->GetContentDir();

    mStyleSet->SetContentRoot(pluginContentPath);
    mStyleSet->SetCoreContentRoot(pluginContentPath);

    //
    //  Default values:

    FVector2D iconSizeMode(40.f, 40.f);
    FVector2D iconSizeModeSmall(20.f, 20.f);

    FVector2D iconSizeEditingMode(64.f, 64.f);

    FSlateColor tintCheckedNormal = FSlateColor(FLinearColor(1.0f, 1.0f, 1.0f, 1.0f));
    FSlateColor tintCheckedHovered = FSlateColor(FLinearColor(1.0f, 1.0f, 1.0f, 1.0f));
    FSlateColor tintCheckedPressed = FSlateColor(FLinearColor(1.0f, 1.0f, 1.0f, 1.0f));
    
    FSlateColor tintUncheckedNormal = FSlateColor(FLinearColor(0.7f, 0.7f, 0.7f, 1.0f));
    FSlateColor tintUncheckedHovered = FSlateColor(FLinearColor(1.0f, 0.9f, 0.6f, 1.0f));
    FSlateColor tintUncheckedPressed = FSlateColor(FLinearColor(1.0f, 0.9f, 0.4f, 1.0f));

    //
    //  Add styles:

    {
        mStyleSet->Set("TileMapEditorMode", new FSlateImageBrush(pluginResourcesPath / TEXT("IconTileMapEditor256.png"), iconSizeMode));
        mStyleSet->Set("TileMapEditorMode.Small", new FSlateImageBrush(pluginResourcesPath / TEXT("IconTileMapEditor256.png"), iconSizeModeSmall));
    }

    {
        FString iconPath = pluginResourcesPath / TEXT("IconPickTile256.png");
        FCheckBoxStyle checkBoxStyle = FCheckBoxStyle()
            .SetCheckedImage(FSlateImageBrush(iconPath, iconSizeEditingMode, tintCheckedNormal))
            .SetCheckedHoveredImage(FSlateImageBrush(iconPath, iconSizeEditingMode, tintCheckedHovered))
            .SetCheckedPressedImage(FSlateImageBrush(iconPath, iconSizeEditingMode, tintCheckedPressed))
            .SetUncheckedImage(FSlateImageBrush(iconPath, iconSizeEditingMode, tintUncheckedNormal))
            .SetUncheckedHoveredImage(FSlateImageBrush(iconPath, iconSizeEditingMode, tintUncheckedHovered))
            .SetUncheckedPressedImage(FSlateImageBrush(iconPath, iconSizeEditingMode, tintUncheckedPressed));
        mStyleSet->Set("TileMapEditor.EditingMode.PickTile", checkBoxStyle);
    }

    {
        FString iconPath = pluginResourcesPath / TEXT("IconResetTile256.png");
        FCheckBoxStyle checkBoxStyle = FCheckBoxStyle()
            .SetCheckedImage(FSlateImageBrush(iconPath, iconSizeEditingMode, tintCheckedNormal))
            .SetCheckedHoveredImage(FSlateImageBrush(iconPath, iconSizeEditingMode, tintCheckedHovered))
            .SetCheckedPressedImage(FSlateImageBrush(iconPath, iconSizeEditingMode, tintCheckedPressed))
            .SetUncheckedImage(FSlateImageBrush(iconPath, iconSizeEditingMode, tintUncheckedNormal))
            .SetUncheckedHoveredImage(FSlateImageBrush(iconPath, iconSizeEditingMode, tintUncheckedHovered))
            .SetUncheckedPressedImage(FSlateImageBrush(iconPath, iconSizeEditingMode, tintUncheckedPressed));
        mStyleSet->Set("TileMapEditor.EditingMode.ResetTile", checkBoxStyle);
    }

    {
        FString iconPath = pluginResourcesPath / TEXT("IconDrawLowObstacle256.png");
        FCheckBoxStyle checkBoxStyle = FCheckBoxStyle()
            .SetCheckedImage(FSlateImageBrush(iconPath, iconSizeEditingMode, tintCheckedNormal))
            .SetCheckedHoveredImage(FSlateImageBrush(iconPath, iconSizeEditingMode, tintCheckedHovered))
            .SetCheckedPressedImage(FSlateImageBrush(iconPath, iconSizeEditingMode, tintCheckedPressed))
            .SetUncheckedImage(FSlateImageBrush(iconPath, iconSizeEditingMode, tintUncheckedNormal))
            .SetUncheckedHoveredImage(FSlateImageBrush(iconPath, iconSizeEditingMode, tintUncheckedHovered))
            .SetUncheckedPressedImage(FSlateImageBrush(iconPath, iconSizeEditingMode, tintUncheckedPressed));
        mStyleSet->Set("TileMapEditor.EditingMode.DrawLowObstacle", checkBoxStyle);
    }

    {
        FString iconPath = pluginResourcesPath / TEXT("IconUnDrawLowObstacle256.png");
        FCheckBoxStyle checkBoxStyle = FCheckBoxStyle()
            .SetCheckedImage(FSlateImageBrush(iconPath, iconSizeEditingMode, tintCheckedNormal))
            .SetCheckedHoveredImage(FSlateImageBrush(iconPath, iconSizeEditingMode, tintCheckedHovered))
            .SetCheckedPressedImage(FSlateImageBrush(iconPath, iconSizeEditingMode, tintCheckedPressed))
            .SetUncheckedImage(FSlateImageBrush(iconPath, iconSizeEditingMode, tintUncheckedNormal))
            .SetUncheckedHoveredImage(FSlateImageBrush(iconPath, iconSizeEditingMode, tintUncheckedHovered))
            .SetUncheckedPressedImage(FSlateImageBrush(iconPath, iconSizeEditingMode, tintUncheckedPressed));
        mStyleSet->Set("TileMapEditor.EditingMode.UndrawLowObstacle", checkBoxStyle);
    }

    {
        FString iconPath = pluginResourcesPath / TEXT("IconDrawHighObstacle256.png");
        FCheckBoxStyle checkBoxStyle = FCheckBoxStyle()
            .SetCheckedImage(FSlateImageBrush(iconPath, iconSizeEditingMode, tintCheckedNormal))
            .SetCheckedHoveredImage(FSlateImageBrush(iconPath, iconSizeEditingMode, tintCheckedHovered))
            .SetCheckedPressedImage(FSlateImageBrush(iconPath, iconSizeEditingMode, tintCheckedPressed))
            .SetUncheckedImage(FSlateImageBrush(iconPath, iconSizeEditingMode, tintUncheckedNormal))
            .SetUncheckedHoveredImage(FSlateImageBrush(iconPath, iconSizeEditingMode, tintUncheckedHovered))
            .SetUncheckedPressedImage(FSlateImageBrush(iconPath, iconSizeEditingMode, tintUncheckedPressed));
        mStyleSet->Set("TileMapEditor.EditingMode.DrawHighObstacle", checkBoxStyle);
    }

    {
        FString iconPath = pluginResourcesPath / TEXT("IconUnDrawHighObstacle256.png");
        FCheckBoxStyle checkBoxStyle = FCheckBoxStyle()
            .SetCheckedImage(FSlateImageBrush(iconPath, iconSizeEditingMode, tintCheckedNormal))
            .SetCheckedHoveredImage(FSlateImageBrush(iconPath, iconSizeEditingMode, tintCheckedHovered))
            .SetCheckedPressedImage(FSlateImageBrush(iconPath, iconSizeEditingMode, tintCheckedPressed))
            .SetUncheckedImage(FSlateImageBrush(iconPath, iconSizeEditingMode, tintUncheckedNormal))
            .SetUncheckedHoveredImage(FSlateImageBrush(iconPath, iconSizeEditingMode, tintUncheckedHovered))
            .SetUncheckedPressedImage(FSlateImageBrush(iconPath, iconSizeEditingMode, tintUncheckedPressed));
        mStyleSet->Set("TileMapEditor.EditingMode.UndrawHighObstacle", checkBoxStyle);
    }

    {
        FString iconPath = pluginResourcesPath / TEXT("IconDrawFire256.png");
        FCheckBoxStyle checkBoxStyle = FCheckBoxStyle()
            .SetCheckedImage(FSlateImageBrush(iconPath, iconSizeEditingMode, tintCheckedNormal))
            .SetCheckedHoveredImage(FSlateImageBrush(iconPath, iconSizeEditingMode, tintCheckedHovered))
            .SetCheckedPressedImage(FSlateImageBrush(iconPath, iconSizeEditingMode, tintCheckedPressed))
            .SetUncheckedImage(FSlateImageBrush(iconPath, iconSizeEditingMode, tintUncheckedNormal))
            .SetUncheckedHoveredImage(FSlateImageBrush(iconPath, iconSizeEditingMode, tintUncheckedHovered))
            .SetUncheckedPressedImage(FSlateImageBrush(iconPath, iconSizeEditingMode, tintUncheckedPressed));
        mStyleSet->Set("TileMapEditor.EditingMode.DrawFire", checkBoxStyle);
    }

    {
        FString iconPath = pluginResourcesPath / TEXT("IconUnDrawFire256.png");
        FCheckBoxStyle checkBoxStyle = FCheckBoxStyle()
            .SetCheckedImage(FSlateImageBrush(iconPath, iconSizeEditingMode, tintCheckedNormal))
            .SetCheckedHoveredImage(FSlateImageBrush(iconPath, iconSizeEditingMode, tintCheckedHovered))
            .SetCheckedPressedImage(FSlateImageBrush(iconPath, iconSizeEditingMode, tintCheckedPressed))
            .SetUncheckedImage(FSlateImageBrush(iconPath, iconSizeEditingMode, tintUncheckedNormal))
            .SetUncheckedHoveredImage(FSlateImageBrush(iconPath, iconSizeEditingMode, tintUncheckedHovered))
            .SetUncheckedPressedImage(FSlateImageBrush(iconPath, iconSizeEditingMode, tintUncheckedPressed));
        mStyleSet->Set("TileMapEditor.EditingMode.UndrawFire", checkBoxStyle);
    }

    //

    FSlateStyleRegistry::RegisterSlateStyle(*mStyleSet.Get());
}

void FTileMapEditorStyle::Shutdown()
{
    if(mStyleSet.IsValid() )
    {
        FSlateStyleRegistry::UnRegisterSlateStyle(*mStyleSet.Get());
        ensure(mStyleSet.IsUnique());
        mStyleSet.Reset();
    }
}

TSharedPtr<FSlateStyleSet> FTileMapEditorStyle::Get()
{
    return mStyleSet;
}

FName FTileMapEditorStyle::GetStyleSetName()
{
    static FName sStyleName("TileMapEditorStyle");
    return sStyleName;
}

const FSlateBrush* FTileMapEditorStyle::GetBrush(FName propertyName,
                                                 const ANSICHAR* specifier)
{
    return mStyleSet->GetBrush(propertyName, specifier);
}

const FCheckBoxStyle& FTileMapEditorStyle::GetCheckBoxStyle(FName propertyName,
                                                            const ANSICHAR* specifier)
{
    return mStyleSet->GetWidgetStyle<FCheckBoxStyle>(propertyName, specifier);
}
