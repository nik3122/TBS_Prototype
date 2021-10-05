// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Version.h"
#include "TileMaps/HexTileGrid.h"
#include "PassMapData.h"
#include "ArenaState.h"
#include "OccupationSlot.h"
#include "PassMap.generated.h"


class APrototypeGameModeBase;
class UTiledMovement;
class UTrapSystem;
class ATurnsManager;
class UStaticMeshComponent;

//  Only for helpers:
class ACreatureBase;


struct State_PassMap
    : public State_Base
{
    DECLARE_CLASS_VERSION(1, 0);
    int32 mSizeX;
    int32 mSizeY;
    uint8* mpTilesOccupations;

    struct OccupiedTile
    {
        int32 mTileIndex;
        uint8 mOccupationMask;
    };
    OccupiedTile* mpOccupiedTiles;
    int32 mOccupiedTilesCount;
};


/**
    Passability map of arena.

    Manages all movements on the passability map.
    
    Width is counted by Y local coordinate.
    Height is counted by X local coordinate.

    All functions works in local space. It is worth to align local coordinate system
    to the world to work in the same coordinates at once.
*/
UCLASS()
class PROTOTYPE_API APassMap
    : public AActor
    , public ArenaStateObject_Base<State_PassMap>
{
    CHECK_CLASS_VERSION_EQUAL(State_PassMap, 1, 0);

    GENERATED_BODY()

    using ArenaHexTileGrid = HexTileGrid<static_cast<uint8>(EOccupationSlot::OccupationSlot_MAX)>;

public:

    static constexpr float sUnitScaleRadius = 100.f;

public:

    DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FEventTileOccupied, const FIntPoint&, tile, EOccupationSlot, occupationSlot, AActor*, pOccupant);
    UPARAM(VisibleAnywhere, BlueprintReadWrite, Category = "TileMap|Arena")
    FEventTileOccupied mEventTileOccupied;

    DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FEventTileReleased, const FIntPoint&, tile, EOccupationSlot, occupationSlot, AActor*, pOccupant);
    UPARAM(VisibleAnywhere, BlueprintReadWrite, Category = "TileMap|Arena")
    FEventTileReleased mEventTileReleased;

    //  For use by FireSystem:
    DECLARE_DELEGATE_OneParam(FEventPassMapMovementRegistered, UTiledMovement*)
    FEventPassMapMovementRegistered mEventPassMapMovementRegistered;

public:

    APassMap(const FObjectInitializer& objectInitializer = FObjectInitializer::Get());

    void BeginPlay() override;

    void SetupSystem(ATurnsManager* pTurnsManager);

    void RegisterPassMapMovement(UTiledMovement* pTiledMovement);

    void UnregisterPassMapMovement(UTiledMovement* pTiledMovement);

    const TSet<UTiledMovement*>& GetRegisteredPassMapMovements() const;

    UTrapSystem* GetTrapSystem() const;

    bool LineTraceTile(const FVector& traceStart,
                       const FVector& traceEnd,
                       FIntPoint& outTile) const;

    const FIntPoint* GetNeighborShifts(int32 parity) const;

    UFUNCTION(BlueprintPure, Category = "TileMap|Arena")
    void GetSpawnTiles(TArray<FIntPoint>& spawnTilesPlayerParty,
                       TArray<FIntPoint>& spawnTilesAIParty) const;

    ArenaHexTileGrid::NeighborsContainer Neighbors(const FIntPoint& tile) const;

    UPassMapData* GetPassMapData() const;

    UFUNCTION(BlueprintPure, Category = "TileMap|Arena")
    int32 GetWidth() const;

    UFUNCTION(BlueprintPure, Category = "TileMap|Arena")
    int32 GetHeight() const;

    UFUNCTION(BlueprintPure, Category = "TileMap|Arena")
    float GetWidthPhysical() const;

    UFUNCTION(BlueprintPure, Category = "TileMap|Arena")
    float GetHeightPhysical() const;

    //  (UGLY) It is made for BP children because it is better not to share FTileData with BP as it will be overhead. for each simple FTileData structure.
    UFUNCTION(BlueprintPure, Category = "TileMap|Arena")
    int32 GetTilesNumber() const;

    UFUNCTION(BlueprintPure, Category = "TileMap|Arena")
    float GetTileRadius() const;

    UFUNCTION(BlueprintPure, Category = "TileMap|Arena")
    FVector GetTilePositionLocal(const FIntPoint& tile) const;

    UFUNCTION(BlueprintPure, Category = "TileMap|Arena")
    FVector GetTilePositionWorld(const FIntPoint& tile) const;

    UFUNCTION(BlueprintPure, Category = "TileMap|Arena")
    FIntPoint GetTileInPositionLocal(const FVector& position) const;

    UFUNCTION(BlueprintPure, Category = "TileMap|Arena")
    FIntPoint GetTileInPositionWorld(const FVector& position) const;

    UFUNCTION(BlueprintPure, Category = "TileMap|Arena")
    FIntPoint CoordFromIndex(int32 index) const;

    UFUNCTION(BlueprintPure, Category = "TileMap|Arena")
    int32 IndexFromCoord(const FIntPoint& coord) const;

    FIntPoint InvalidTile() const;

    UFUNCTION(BlueprintPure, Category = "TileMap|Arena")
    bool IsTileValid(const FIntPoint& tile) const;

    UFUNCTION(BlueprintPure, Category = "TileMap|Arena")
    bool IsNeighbour(const FIntPoint& isTile,
                     const FIntPoint& ofTile) const;

    UFUNCTION(BlueprintPure, Category = "TileMap|Arena")
    FIntPoint GetNeighbour(const FIntPoint& tile,
                           int32 direction) const;

    //  Get direction from targetTile to neighborTile.
    UFUNCTION(BlueprintPure, Category = "TileMap|Arena")
    int32 GetNeighbourDirection(const FIntPoint& neighbourTile,
                                const FIntPoint& targetTile) const;

    UFUNCTION(BlueprintPure, Category = "TileMap|Arena")
    int32 GetDirectionFromVector(const FVector& vector) const;

    UFUNCTION(BlueprintPure, Category = "TileMap|Arena")
    int32 GetDirectionBetweenTiles(const FIntPoint& fromTile,
                                   const FIntPoint& toTile) const;

    //  Get rotation corresponding to the tile direction.
    UFUNCTION(BlueprintPure, Category = "TileMap|Arena")
    FRotator GetDirectionRotation(int32 tileDirection) const;

    UFUNCTION(BlueprintPure, Category = "TileMap|Arena")
    bool CheckNeighborhood(const FIntPoint& targetTile,
                           const FIntPoint& neighbourTile,
                           int32& direction) const;

    UFUNCTION(BlueprintPure, Category = "TileMap|Arena")
    bool IsNeighbourDirectionValid(int32 direction) const;

    UFUNCTION(BlueprintPure, Category = "TileMap|Arena")
    int32 GetDistance(const FIntPoint& tileA,
                      const FIntPoint& tileB) const;

    UFUNCTION(BlueprintPure, Category = "TileMap|Arena")
    bool IsTileFree(const FIntPoint& tile,
                    int32 occupationSlotsToBeFree,
                    bool atLeastOne = false) const;

    UFUNCTION(BlueprintPure, Category = "TileMap|Arena")
    bool IsTileFree_Safe(const FIntPoint& tile,
                         int32 occupationSlotsToBeFree,
                         bool atLeastOne = false) const;

    UFUNCTION(BlueprintCallable, Category = "TileMap|Arena")
    void OccupyTile(const FIntPoint& tile,
                    EOccupationSlot occupationSlot,
                    AActor* pOccupant);

    UFUNCTION(BlueprintCallable, Category = "TileMap|Arena")
    void OccupyTile_Safe(const FIntPoint& tile,
                         EOccupationSlot occupationSlot,
                         AActor* pOccupant);

    UFUNCTION(BlueprintCallable, Category = "TileMap|Arena")
    AActor* FreeTile(const FIntPoint& tile,
                     EOccupationSlot occupationSlot);

    UFUNCTION(BlueprintCallable, Category = "TileMap|Arena")
    AActor* FreeTile_Safe(const FIntPoint& tile,
                          EOccupationSlot occupationSlot);

    UFUNCTION(BlueprintCallable, Category = "TileMap|Arena")
    void ResetAllOccupationsAtSlot(EOccupationSlot occupationSlot,
                                   bool skipNotifications);

    UFUNCTION(BlueprintPure, Category = "TileMap|Arena")
    bool IsTileFreeAtSlot(const FIntPoint& tile,
                          EOccupationSlot occupationSlot) const;

    UFUNCTION(BlueprintPure, Category = "TileMap|Arena")
    bool IsTileFreeAtSlot_Safe(const FIntPoint& tile,
                               EOccupationSlot occupationSlot) const;

    UFUNCTION(BlueprintPure, Category = "TileMap|Arena")
    AActor* GetOccupant(const FIntPoint& tile,
                        EOccupationSlot occupationSlot) const;

    UFUNCTION(BlueprintPure, Category = "TileMap|Arena")
    AActor* GetOccupant_Safe(const FIntPoint& tile,
                             EOccupationSlot occupationSlot) const;

    /**
        Searches the path between two tiles.
        If the path between tiles exists, the shortest path is returned.
        lengthLimit - gives maximum length of the path which can be found. -1 means no limit.
        outputLimit - the maximum number of output tiles if the path is found. -1 means output full path.
        ignoreDynamicChange - tells if (false) the algorithm should count dynamicly occupied tiles as free
        with decreased weight, or (true) should count them as occupied and will ignore them.
    */
    UFUNCTION(BlueprintCallable, Category = "TileMap|Arena")
    TArray<FIntPoint> FindPath(const FIntPoint& from,
                               const FIntPoint& to,
                               int32 occupationSlotsToBeFree,
                               int32 occupationSlotsInterruption,
                               int32 depthLimit = -1,
                               int32 outputLimit = -1,
                               bool ignoreTargetTileOccupation = false);

    /**
        Detects if direct visibility exists between two tiles.
        If no tile is blocked on the way of direct visibility, the target (to) tile
        is returned. Otherwise a blocking tile is returned.
        Raycast step should be > 0.0f.
    */
    UFUNCTION(BlueprintPure, Category = "TileMap|Arena")
    bool DirectVisibility(const FIntPoint& from,
                          const FIntPoint& to,
                          int32 occupationSlotsToBeFree,
                          FIntPoint& outBlockingTile) const;

    UFUNCTION(BlueprintCallable, Category = "TileMap|Arena")
    int32 GetLine(const FIntPoint& from,
                  const FIntPoint& to,
                  TArray<FIntPoint>& outTilesOfLine,
                  float raycastStep = -0.5f);

    UFUNCTION(BlueprintCallable, Category = "TileMap|Arena")
    int32 GetReachableTiles(const FIntPoint& centralTile,
                            int32 depthLimit,
                            int32 occupationSlotsToBeFree,
                            TArray<FIntPoint>& outReachableTiles);

    UFUNCTION(BlueprintCallable, Category = "TileMap|Arena")
    int32 GetHexaArea(const FIntPoint& centralTile,
                      int32 areaRadius,
                      TArray<FIntPoint>& outAreaTiles);

    UFUNCTION(BlueprintCallable, Category = "TileMap|Arena")
    int32 GetHexaFrame(const FIntPoint& centralTile,
                       int32 frameRadius,
                       TArray<FIntPoint>& outAreaTiles);

    //  3 sectors marked with their directions:
    //    /   \___/   \___/ 0 \___/ 0 \___/ 0 \
    //    \___/   \___/   \___/ 0 \___/ 0 \___/
    //    / 4 \___/   \___/ 0 \___/ 0 \___/ 0 \
    //    \___/ 4 \___/   \___/ 0 \___/ 0 \___/
    //    / 4 \___/ 4 \___/ 0 \___/ 0 \___/   \
    //    \___/ 4 \___/ 4 \___/ 0 \___/   \___/
    //    / 4 \___/ 4 \___/ÑNT\___/   \___/   \
    //    \___/ 4 \___/ 4 \___/ 2 \___/   \___/
    //    / 4 \___/ 4 \___/ 2 \___/ 2 \___/   \
    //    \___/ 4 \___/   \___/ 2 \___/ 2 \___/
    //    / 4 \___/   \___/ 2 \___/ 2 \___/ 2 \
    //    \___/   \___/   \___/ 2 \___/ 2 \___/
    //    /   \___/   \___/ 2 \___/ 2 \___/ 2 \
    //    \___/   \___/   \___/   \___/   \___/
    UFUNCTION(BlueprintCallable, Category = "TileMap|Arena")
    int32 GetHexaSectors(const FIntPoint& centralTile,
                         int32 sectorRadius,
                         int32 firstSectorDirection,
                         int32 lastSectorDirection,
                         TArray<FIntPoint>& outAreaTiles);

    UFUNCTION(BlueprintCallable, Category = "TileMap|Arena")
    int32 GetHexaRing(const FIntPoint& centralTile,
                      int32 ringRadiusMin,
                      int32 ringRadiusMax,
                      TArray<FIntPoint>& outRingTiles);

    UFUNCTION(BlueprintCallable, Category = "TileMap|Arena")
    void ForEachTileInHexaArea(const FIntPoint& centralTile,
                               int32 frameRadius,
                               TScriptInterface<ITileOperator_Interface> tileOperator,
                               bool includingCentralTile = false);

    UFUNCTION(BlueprintCallable, Category = "TileMap|Arena")
    void ForEachTileInHexaFrame(const FIntPoint& centralTile,
                                int32 frameRadius,
                                TScriptInterface<ITileOperator_Interface> tileOperator);

    UFUNCTION(BlueprintCallable, Category = "TileMap|Arena")
    int32 GetRandomFreeTilesInHexaArea(const FIntPoint& centralTile,
                                       int32 areaRadius,
                                       int32 maxFreeTilesCount,
                                       int32 occupationSlotsToBeFree,
                                       TArray<FIntPoint>& outRandomFreeTiles);

    UFUNCTION(BlueprintCallable, Category = "TileMap|Arena")
    void ForEachTileInSectors(const FIntPoint& centralTile,
                              int32 areaRadius,
                              int32 firstSectorDirection,
                              int32 lastSectorDirection,
                              TScriptInterface<ITileOperator_Interface> tileOperator,
                              bool includingCentralTile = false);

    UFUNCTION(BlueprintCallable, Category = "TileMap|Arena")
    void GetRandomTilesChain(const FIntPoint& chainStartTile,
                             int32 chainLength,
                             const TArray<FIntPoint>& tilesToIgnore,
                             TArray<FIntPoint>& outChainTiles);

    UFUNCTION(BlueprintCallable, Category = "TileMap|Arena")
    int32 GetOccupantsInHexaArea(const FIntPoint& centralTile,
                                 int32 areaRadius,
                                 EOccupationSlot occupationSlot,
                                 TArray<AActor*>& outOccupants);

    UFUNCTION(BlueprintCallable, Category = "TileMap|Arena")
    int32 GetOccupantsInHexaFrame(const FIntPoint& centralTile,
                                  int32 frameRadius,
                                  EOccupationSlot occupationSlot,
                                  TArray<AActor*>& outOccupants);
    
    UFUNCTION(BlueprintCallable, Category = "TileMap|Arena")
    int32 GetOccupantsInSectors(const FIntPoint& centralTile,
                                int32 areaRadius,
                                int32 firstSectorDirection,
                                int32 lastSectorDirection,
                                EOccupationSlot occupationSlot,
                                TArray<AActor*>& outOccupants);

    //  Helpers:

    UFUNCTION(BlueprintCallable, Category = "TileMap|Arena")
    void SortCreaturesByDistanceToTile(UPARAM(ref) TArray<ACreatureBase*>& creatures, FIntPoint tile, UPARAM(ref) TArray<int32>& distances);

#if WITH_EDITOR

    void PostEditChangeProperty(FPropertyChangedEvent& propertyChangedEvent) override;

    void SetTileMapData(UPassMapData* passMapData);

    void OnTileDataModified(const FIntPoint& modifiedTile);

    void OnPassMapDataUpdated();

#endif // #if WITH_EDITOR

protected:

    //	This variable is just for initialization and can be replaced with setter.
    UPROPERTY(EditAnywhere, Category = "TileMap", Meta = (ClampMin = 1, EditCondition = "mpData == 0"))
    int32 mWidth = 1;

    //	This variable is just for initialization and can be replaced with setter.
    UPROPERTY(EditAnywhere, Category = "TileMap", Meta = (ClampMin = 1, EditCondition = "mpData == 0"))
    int32 mHeight = 1;

    UPROPERTY(EditAnywhere, Category = "TileMap")
    float mTileRadius = 100.0f;

    UPROPERTY(EditInstanceOnly, Category = "TileMap|Arena")
    UPassMapData* mpData;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "TileMap|Arena")
    TArray<FIntPoint> mSpawnTilesPlayerParty;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "TileMap|Arena")
    TArray<FIntPoint> mSpawnTilesAIParty;

    UPROPERTY(BlueprintReadOnly, Category = "TileMap|Arena")
    UTrapSystem* mpTrapSystem = nullptr;

    ArenaHexTileGrid mHexTileGrid;

    TSet<UTiledMovement*> mRegisteredPassMapMovements;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "TileMap")
    UStaticMeshComponent* mSimplifiedTraceComponent = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Transient, Category = "TileMap")
    UInstancedStaticMeshComponent* mTilesInstances = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Transient, Category = "TileMap")
    UInstancedStaticMeshComponent* mFirePlatesInstances = nullptr;

protected:

    void BuildTileGrid();

    UFUNCTION(BlueprintImplementableEvent, Category = "TileMap|Arena", Meta = (DisplayName = "Init Tile Grid"))
    void K2_InitTileGrid();

    UFUNCTION()
    void OnFinalizeTurn();

    void OnConstruction(const FTransform& Transform) override;

    void StoreState_Inner(ArenaStateContext& context, State_PassMap& state) const override;

    bool RestoreState_Inner(const State_PassMap& state) override;
    
    void OnTileOccupied(const FIntPoint& tile, uint8 occupationSlot, AActor* pOccupant);

    void OnTileReleased(const FIntPoint& tile, uint8 occupationSlot, AActor* pOccupant);

    UPrimitiveComponent* GetTraceComponent() const;

private:

    void SetTileData(const FIntPoint& tile,
                     const FTileParameters& tileData);

    void RebuildVisuals();

    void RebuildVisuals(const FIntPoint& modifiedTile);

    void GetTileTransformLocal(const FIntPoint& tile,
                               FTransform& outTileTransform,
                               FTransform& outFirePlateTransform);
};
