

#include "EdMode_TileMap.h"
#include "AssetToolsModule.h"
#include "Engine/Selection.h"
#include "EditorModeManager.h"
#include "EditorViewportClient.h"
#include "EdModeToolkit_TileMap.h"
#include "IAssetTools.h"
#include "Modules/ModuleManager.h"
#include "Misc/PackageName.h"
#include "Arena/PassMap.h"
#include "Arena/PassMapData.h"
#include "TileMapEditorCommon.h"
#include "Toolkits/ToolkitManager.h"
#include "UObject/UObjectBaseUtility.h"


const FEditorModeID FEdMode_TileMap::EM_TileMapEditorEdModeId = TEXT("EM_TileMapEditorEdMode");

FEdMode_TileMap::FEdMode_TileMap()
{

}

FEdMode_TileMap::~FEdMode_TileMap()
{

}

void FEdMode_TileMap::Enter()
{
    FEdMode::Enter();

    UE_LOG(LogTileMapEditor, Log, TEXT("Entered tile map edditing mode."));

    if (!Toolkit.IsValid() && UsesToolkits())
    {
        Toolkit = MakeShareable(new FEdModeToolkit_TileMap);
        Toolkit->Init(Owner->GetToolkitHost());
    }

    mPassMap.Reset();

    USelection* selection = GEditor->GetSelectedActors();
    TArray<UObject*> selectedTileMaps;
    selection->GetSelectedObjects(APassMap::StaticClass(), selectedTileMaps);
    if (selectedTileMaps.Num() > 0)
    {
        mPassMap = Cast<APassMap>(selectedTileMaps[0]);
    }
    selection->DeselectAll();

    ETileMapEditingMode newEditingMode = ETileMapEditingMode::Invalid;
    if (mPassMap.IsValid())
    {
        selection->Select(mPassMap.Get());

        newEditingMode = mPassMap->GetPassMapData() != nullptr ? ETileMapEditingMode::None : ETileMapEditingMode::Invalid;
    }
    else
    {
        newEditingMode = ETileMapEditingMode::Invalid;
    }
    SetCurrentEditingMode(newEditingMode);
}

void FEdMode_TileMap::Exit()
{
    if (Toolkit.IsValid())
    {
        FToolkitManager::Get().CloseToolkit(Toolkit.ToSharedRef());
        Toolkit.Reset();
    }

    mPassMap.Reset();
    SetCurrentEditingMode(ETileMapEditingMode::Invalid);

    UE_LOG(LogTileMapEditor, Log, TEXT("Exited tile map edditing mode."));

    FEdMode::Exit();
}

void FEdMode_TileMap::Tick(FEditorViewportClient* viewportClient,
                           float deltaTime)
{
    if (IsDrawing())
    {
        HandleDrawing(viewportClient);
    }
}

bool FEdMode_TileMap::UsesToolkits() const
{
    return true;
}

bool FEdMode_TileMap::MouseMove(FEditorViewportClient* viewportClient,
                                FViewport* viewport,
                                int32 x, int32 y)
{
    return false;
}

bool FEdMode_TileMap::HandleClick(FEditorViewportClient* inViewportClient,
                                  HHitProxy* hitProxy,
                                  const FViewportClick& click)
{
    return false;
}

bool FEdMode_TileMap::InputKey(FEditorViewportClient* viewportClient, 
                               FViewport* viewport, 
                               FKey key, 
                               EInputEvent event)
{
    if (mCurrentEditingMode > ETileMapEditingMode::None)
    {
        if (key == EKeys::LeftMouseButton)
        {
            if (event == EInputEvent::IE_Pressed)
            {
                if (IsDrawing())
                {
                    StopDrawing();
                }
                else
                {
                    StartDrawing();
                }
                return true;
            }
            else if (event == EInputEvent::IE_Released &&
                     IsDrawing())
            {
                StopDrawing();
                return true;
            }
        }
    }
    return false;
}

bool FEdMode_TileMap::DisallowMouseDeltaTracking() const
{
    return mCurrentEditingMode > ETileMapEditingMode::PickTile &&
        mCurrentEditingMode <= ETileMapEditingMode::MAX;
}

bool FEdMode_TileMap::Select(AActor* actor,
                             bool isSelected)
{
    if (mCurrentEditingMode == ETileMapEditingMode::Invalid ||
        mCurrentEditingMode == ETileMapEditingMode::None ||
        mCurrentEditingMode == ETileMapEditingMode::PickTile)
    {
        APassMap* selectedPassMap = Cast<APassMap>(actor);
        if (selectedPassMap != nullptr)
        {
            if (isSelected)
            {
                if (selectedPassMap != mPassMap)
                {
                    SetPassMap(selectedPassMap);
                    return false;
                }
            }
            else
            {
                if (selectedPassMap == mPassMap)
                {
                    SetPassMap(nullptr);
                    return false;
                }
            }
        }
    }
    return true;
}

bool FEdMode_TileMap::IsSelectionAllowed(AActor* actor,
                                         bool isInSelection) const
{

    return actor->IsA<APassMap>() &&
        (mCurrentEditingMode == ETileMapEditingMode::Invalid ||
         mCurrentEditingMode == ETileMapEditingMode::None ||
         mCurrentEditingMode == ETileMapEditingMode::PickTile);
}

bool FEdMode_TileMap::AllowsViewportDragTool() const
{
    return false;
}

bool FEdMode_TileMap::UsesTransformWidget() const
{
    return false;
}

bool FEdMode_TileMap::ShouldDrawWidget() const
{
    return true;
}

APassMap* FEdMode_TileMap::GetTileMap() const
{
    return mPassMap.Get();
}

void FEdMode_TileMap::RefreshDetailPanel()
{
    if (Toolkit.IsValid())
    {
        StaticCastSharedPtr<FEdModeToolkit_TileMap>(Toolkit)->RefreshDetailPanel();
    }
}

bool FEdMode_TileMap::SetCurrentEditingMode(ETileMapEditingMode editingMode)
{
    if (editingMode == mCurrentEditingMode)
    {
        return true;
    }

    if (editingMode == ETileMapEditingMode::Invalid)
    {
        mCurrentEditingMode = ETileMapEditingMode::Invalid;
        UE_LOG(LogTileMapEditor, Warning, TEXT("Invalidating editing mode."));
        return true;
    }

    if (mPassMap->GetPassMapData() == nullptr)
    {
        mCurrentEditingMode = ETileMapEditingMode::Invalid;
        UE_LOG(LogTileMapEditor, Warning, TEXT("Can't switch editing mode as no pass map data is specified with pass map\"%s\"."));
        return false;
    }

    mCurrentEditingMode = editingMode;

    UE_LOG(LogTileMapEditor, Warning, TEXT("New editing mode is set: %s"),
           ToString(mCurrentEditingMode));

    return true;
}

ETileMapEditingMode FEdMode_TileMap::GetCurrentEditingMode() const
{
    return mCurrentEditingMode;
}

FIntPoint FEdMode_TileMap::GetTileUnderCursor() const
{
    return mTileUnderCursor;
}

UPassMapData* FEdMode_TileMap::CreateTileMapDataForSelectedTileMap()
{
    if (mPassMap == nullptr ||
        mPassMap->GetPassMapData() != nullptr)
    {
        UE_LOG(LogTileMapEditor, Warning, TEXT("PassMap is already associated with PassMapData. Can't create new data."));
        return nullptr;
    }

    UPackage* levelPackage = mPassMap->GetPackage();
    FString levelPackageFullPath = levelPackage->GetPathName();
    FString levelPackagePath = FPackageName::GetLongPackagePath(levelPackageFullPath);
    FString levelName = FPackageName::GetShortName(levelPackageFullPath);
    int32 prefixEndIndex = INDEX_NONE;
    levelName.FindChar('_', prefixEndIndex);
    levelName.RightChopInline(prefixEndIndex + 1, false);

    //  ~TODO~ Create pass map asset object.
    //  Bind pass map asset to the selected pass map.

    FAssetToolsModule& assetToolsModule = FModuleManager::Get().LoadModuleChecked<FAssetToolsModule>("AssetTools");
    IAssetTools& assetTools = assetToolsModule.Get();
    
    FString passMapDataName = TEXT("TILES_") + levelName;
    auto newPassMapData = Cast<UPassMapData>(assetTools.CreateAsset(passMapDataName,
                                                                    levelPackagePath,
                                                                    UPassMapData::StaticClass(),
                                                                    nullptr));
    if (newPassMapData != nullptr)
    {
        UE_LOG(LogTileMapEditor, Log, TEXT("Created new PassMapData asset \"%s\"."),
               *(levelPackagePath / passMapDataName));
        
        //  Initialize newly create PassMapData:
        newPassMapData->mWidth = 10;
        newPassMapData->mHeight = 10;
        newPassMapData->mTiles.SetNum(newPassMapData->mWidth * newPassMapData->mHeight);

        mPassMap->SetTileMapData(newPassMapData);
        mPassMap->Modify();

        SetCurrentEditingMode(ETileMapEditingMode::None);
    }
    else
    {
        UE_LOG(LogTileMapEditor, Error, TEXT("Failed to create PassMapData asset \"%s\"."),
               *(levelPackagePath / passMapDataName));
    }

    return newPassMapData;
}

void FEdMode_TileMap::ResizeTileMap(int32 newWidth, int32 newHeight)
{
    if (newWidth <= 0 ||
        newHeight <= 0)
    {
        UE_LOG(LogTileMapEditor, Warning, TEXT("Can't resize tile map. Specified invalid new size [%i; i%]."),
               newHeight,
               newWidth);
        return;
    }

    if (mPassMap == nullptr ||
        mPassMap->GetPassMapData() == nullptr)
    {
        return;
    }

    //  ResizeTileMap storing the data in the cache.
    UPassMapData* passMapData = mPassMap->GetPassMapData();
    int32 oldWidth = passMapData->mWidth;
    int32 oldHeight = passMapData->mHeight;

    if (oldWidth == newWidth &&
        oldHeight == newHeight)
    {
        return;
    }

    TArray<FTileParameters> oldTiles = passMapData->mTiles;
    TArray<FTileParameters>& newTiles = passMapData->mTiles;

    passMapData->mWidth = newWidth;
    passMapData->mHeight = newHeight;
    passMapData->mTiles.SetNumZeroed(newWidth * newHeight, true);

    for (int32 x = 0; x < FMath::Min(oldHeight, newHeight); ++x)
    {
        for (int32 y = 0; y < FMath::Min(oldWidth, newWidth); ++y)
        {
            FIntPoint tile(x, y);
            int32 oldIndex = passMapData->IndexFromCoord(tile, oldWidth, oldHeight);
            int32 newIndex = passMapData->IndexFromCoord(tile);
            newTiles[newIndex] = oldTiles[oldIndex];
        }
    }

    passMapData->Modify();

    mPassMap->OnPassMapDataUpdated();
    
}

void FEdMode_TileMap::SetPassMap(APassMap* newPassMap)
{
    if (mPassMap == newPassMap)
    {
        return;
    }

    if (mPassMap.IsValid())
    {
        UE_LOG(LogTileMapEditor, Log, TEXT("Stop edditing pass map \"%s\"."),
               *mPassMap->GetName());
        //  ~TODO~ Save all modifications of currently selected pass map.
    }

    mPassMap = newPassMap;

    if (mPassMap.IsValid())
    {
        UE_LOG(LogTileMapEditor, Log, TEXT("Start edditing pass map \"%s\"."),
               *mPassMap->GetName());
        //  ~TODO~ New pass map being selected. Prepare it for editing.
        SetCurrentEditingMode(ETileMapEditingMode::None);
    }
    else
    {
        SetCurrentEditingMode(ETileMapEditingMode::Invalid);
    }
}

void FEdMode_TileMap::StartDrawing()
{
    check(mIsDrawing == false);

    mIsDrawing = true;

    UE_LOG(LogTileMapEditor, Log, TEXT("Start drawing."));
}

void FEdMode_TileMap::StopDrawing()
{
    check(mIsDrawing);

    mIsDrawing = false;

    UE_LOG(LogTileMapEditor, Log, TEXT("Stop drawing."));
}

bool FEdMode_TileMap::IsDrawing()
{
    return mIsDrawing;
}

void FEdMode_TileMap::HandleDrawing(FEditorViewportClient* viewportClient)
{
    if (mCurrentEditingMode != ETileMapEditingMode::Invalid &&
        mCurrentEditingMode != ETileMapEditingMode::None &&
        mCurrentEditingMode != ETileMapEditingMode::PickTile &&
        mPassMap.IsValid())
    {
        UPassMapData* passMapData = mPassMap->GetPassMapData();
        if (passMapData != nullptr)
        {
            FViewportCursorLocation cursorLocation = viewportClient->GetCursorWorldLocationFromMousePos();
            bool tileTraced = mPassMap->LineTraceTile(cursorLocation.GetOrigin(),
                                                      cursorLocation.GetOrigin() + cursorLocation.GetDirection() * 9999.f,
                                                      mTileUnderCursor);
            if (tileTraced)
            {
                FTileParameters* tileParameters = passMapData->GetTileParameters(mTileUnderCursor);
                if (tileParameters != nullptr)
                {
                    bool isModified = false;

                    switch (mCurrentEditingMode)
                    {
                    case ETileMapEditingMode::ResetTile:
                        tileParameters->mIgnitable = false;
                        tileParameters->mStaticLowObstacle = false;
                        tileParameters->mStaticHighObstacle = false;
                        isModified = true;
                        break;

                    case ETileMapEditingMode::DrawLowObstacle:
                        if (tileParameters->mStaticLowObstacle == false)
                        {
                            tileParameters->mStaticLowObstacle = true;
                            tileParameters->mStaticHighObstacle = false;
                            isModified = true;
                        }
                        break;

                    case ETileMapEditingMode::UndrawLowObstacle:
                        if (tileParameters->mStaticLowObstacle == true)
                        {
                            tileParameters->mStaticLowObstacle = false;
                            isModified = true;
                        }
                        break;

                    case ETileMapEditingMode::DrawHighObstacle:
                        if (tileParameters->mStaticHighObstacle == false)
                        {
                            tileParameters->mStaticHighObstacle = true;
                            tileParameters->mStaticLowObstacle = false;
                            isModified = true;
                        }
                        break;

                    case ETileMapEditingMode::UndrawHighObstacle:
                        if (tileParameters->mStaticHighObstacle == true)
                        {
                            tileParameters->mStaticHighObstacle = false;
                            isModified = true;
                        }
                        break;

                    case ETileMapEditingMode::DrawFire:
                        if (tileParameters->mIgnitable == false)
                        {
                            tileParameters->mIgnitable = true;
                            isModified = true;
                        }
                        break;

                    case ETileMapEditingMode::UndrawFire:
                        if (tileParameters->mIgnitable == true)
                        {
                            tileParameters->mIgnitable = false;
                            isModified = true;
                        }
                        break;
                    }

                    if (isModified)
                    {
                        UE_LOG(LogTileMapEditor, Log, TEXT("Modified tile [%i; %i]."),
                               mTileUnderCursor.X,
                               mTileUnderCursor.Y);
                        passMapData->Modify();
                        mPassMap->OnTileDataModified(mTileUnderCursor);
                    }
                }
            }
        }
    }
}
