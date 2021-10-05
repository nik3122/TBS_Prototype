#pragma once


#include "CoreMinimal.h"
#include "Styling/SlateStyle.h"


class FSlateStyleSet;


class FTileMapEditorStyle
{
public:

    static void Initialize();

    static void Shutdown();

    static TSharedPtr<FSlateStyleSet> Get();

    static FName GetStyleSetName();

    static const FSlateBrush* GetBrush(FName propertyName,
                                       const ANSICHAR* specifier = nullptr);

    static const FCheckBoxStyle& GetCheckBoxStyle(FName propertyName,
                                                  const ANSICHAR* specifier = nullptr);

private:

    static TSharedPtr<FSlateStyleSet> mStyleSet;
};
