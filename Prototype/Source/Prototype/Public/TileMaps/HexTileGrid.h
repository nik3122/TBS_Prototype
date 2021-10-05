#pragma once


#include "Arena/Tile.h"
#include "Arena/TileOperator_Interface.h"


#define WITH_HEX_TILE_GRID_DEBUG                        1
#define WITH_HEX_TILE_GRID_LOG                          1
#define WITH_HEX_TILE_GRID_DEBUG_DISPLAY_TIME           10


template <uint8 tOccupantSlotsCount>
class HexTileGrid
{
public:

    DECLARE_DELEGATE_ThreeParams(FEventTileOccupation, const FIntPoint&, uint8, AActor*);

    static const uint32 mNeighborsCount = 6;

    static const FIntPoint mNeighbourShifts[2][mNeighborsCount];

    /**
    Container of neighbors of a tile.
    */
    class NeighborsContainer
    {
    public:
        class Iterator
        {
        public:
            Iterator& operator++(int);
            Iterator& operator++();

            bool operator==(const Iterator& right) const;
            bool operator!=(const Iterator& right) const;

            FIntPoint operator*();
            const FIntPoint& operator*() const;

        private:
            const NeighborsContainer& mNeighbourShifts;
            int32 mIndex;

        private:
            explicit Iterator(NeighborsContainer& neighbors);
            explicit Iterator(NeighborsContainer& neighbors, bool);

        private:
            friend class NeighborsContainer;
        };

    public:
        Iterator begin();
        Iterator end();

    private:
        const HexTileGrid& mTileMap;
        int32 mCount;
        FIntPoint mNeighbourShifts[6];

    private:
        explicit NeighborsContainer(const HexTileGrid& passMap,
                                     int32 neighborsCount,
                                     FIntPoint neighbors[6]);

    private:
        friend class Iterator;
        friend class HexTileGrid;
    };

    friend class NeighborsContainer;

public:

    FDelegateHandle SetDelegateTileOccupied(const FEventTileOccupation& delegate);

    FDelegateHandle SetDelegateTileReleased(const FEventTileOccupation& delegate);

    NeighborsContainer Neighbors(const FIntPoint& tile) const;

    void ResetSize(uint32 width,
                   uint32 height,
                   bool resetData);

    uint32 GetWidth() const;

    uint32 GetHeight() const;

    //	(UGLY) It is made for BP children because it is better not to share FTileData with BP as it will be overhead. for each simple FTileData structure.
    int32 GetTilesNumber() const;

    void SetTileRadius(float tileRadius);

    float GetTileRadius() const;

    const TArray<uint8>& GetOccupationData() const;

    FVector GetTilePosition(const FIntPoint& tile) const;

    FIntPoint GetTileInPosition(const FVector& position) const;

    FIntPoint CoordFromIndex(int32 index) const;

    int32 IndexFromCoord(const FIntPoint& coord) const;

    int32 IndexFromCoord(const FIntPoint& coord, uint32 width, uint32 height) const;

    bool IsTileValid(const FIntPoint& tile) const;

    bool IsNeighbour(const FIntPoint& isTile,
                     const FIntPoint& ofTile) const;

    FIntPoint GetNeighbour(const FIntPoint& tile,
                           int32 direction) const;

    int32 GetNeighbourDirection(const FIntPoint& neighbourTile,
                                const FIntPoint& targetTile) const;

    int32 GetDirectionFromVector(const FVector& vector) const;

    int32 GetDirectionBetweenTiles(const FIntPoint& fromTile,
                                   const FIntPoint& toTile) const;

    bool CheckNeighborhood(const FIntPoint& targetTile,
                           const FIntPoint& neighbourTile,
                           int32& direction) const;

    bool IsNeighbourDirectionValid(int32 direction) const;

    int32 GetDistance(const FIntPoint& tileA,
                      const FIntPoint& tileB) const;

    void OccupyTile(const FIntPoint& tile,
                    uint8 occupationSlot,
                    AActor* pOccupant);

    void OccupyTile_Safe(const FIntPoint& tile,
                         uint8 occupationSlot,
                         AActor* pOccupant);

    AActor* FreeTile(const FIntPoint& tile,
                     uint8 occupationSlot);

    AActor* FreeTile_Safe(const FIntPoint& tile,
                          uint8 occupationSlot);

    void ResetAllOccupations(uint8 occupationSlot,
                             bool skipNotify = true);

    bool IsTileFree(const FIntPoint& tile,
                    int32 occupationSlotsToBeFree,
                    bool atLeastOne = false) const;

    bool IsTileFree_Safe(const FIntPoint& tile,
                         int32 occupationSlotsToBeFree,
                         bool atLeastOne = false) const;

    bool IsTileFreeAtSlot(const FIntPoint& tile,
                          uint8 occupationSlot) const;

    bool IsTileFreeAtSlot_Safe(const FIntPoint& tile,
                               uint8 occupationSlot) const;

    AActor* GetOccupant(const FIntPoint& tile,
                        uint8 occupationSlot) const;

    AActor* GetOccupant_Safe(const FIntPoint& tile,
                             uint8 occupationSlot) const;

    /**
        Searches the path between two tiles.
        If the path between tiles exists, the shortest path is returned.
        lengthLimit - gives maximum length of the path which can be found. -1 means no limit.
        outputLimit - the maximum number of output tiles if the path is found. -1 means output full path.
        ignoreDynamicChange - tells if (false) the algorithm should count dynamicly occupied tiles as free
        with decreased weight, or (true) should count them as occupied and will ignore them.
    */
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
    bool DirectVisibility(const FIntPoint& from,
                          const FIntPoint& to,
                          int32 occupationSlotsToBeFree,
                          float raymarchStepSize,
                          FIntPoint& outBlockingTile) const;

    int32 GetLine(const FIntPoint& from,
                  const FIntPoint& to,
                  TArray<FIntPoint>& outTilesOfLine,
                  float raycastStep = -0.5f);

    int32 GetReachableTiles(const FIntPoint& centralTile,
                            int32 depthLimit,
                            int32 occupationSlotsToBeFree,
                            TArray<FIntPoint>& outReachableTiles);

    int32 GetHexaArea(const FIntPoint& centralTile,
                      int32 areaRadius,
                      TArray<FIntPoint>& outAreaTiles);

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
    int32 GetHexaSectors(const FIntPoint& centralTile,
                         int32 sectorRadius,
                         int32 firstSectorDirection,
                         int32 lastSectorDirection,
                         TArray<FIntPoint>& outAreaTiles);

    int32 GetHexaRing(const FIntPoint& centralTile,
                      int32 ringRadiusMin,
                      int32 ringRadiusMax,
                      TArray<FIntPoint>& outRingTiles);

    template <class Predicate>
    void ForEachTileInHexaArea(const FIntPoint& centralTile,
                               int32 frameRadius,
                               Predicate predicate,
                               bool includingCentralTile = false);

    template <class Predicate>
    void ForEachTileInHexaFrame(const FIntPoint& centralTile,
                                int32 frameRadius,
                                Predicate predicate);

    template <class Predicate>
    void ForEachTileInSectors(const FIntPoint& centralTile,
                              int32 areaRadius,
                              int32 firstSectorDirection,
                              int32 lastSectorDirection,
                              Predicate predicate,
                              bool includingCentralTile = false);

    int32 GetRandomFreeTilesInHexaArea(const FIntPoint& centralTile,
                                       int32 areaRadius,
                                       int32 maxFreeTilesCount,
                                       int32 occupationSlotsToBeFree,
                                       TArray<FIntPoint>& outRandomFreeTiles);

    void GetRandomTilesChain(const FIntPoint& chainStartTile,
                             int32 chainLength,
                             const TArray<FIntPoint>& tilesToIgnore,
                             TArray<FIntPoint>& outChainTiles);

    int32 GetOccupantsInHexaArea(const FIntPoint& centralTile,
                                 int32 areaRadius,
                                 uint8 occupationSlot,
                                 TArray<AActor*>& outOccupants);

    int32 GetOccupantsInHexaFrame(const FIntPoint& centralTile,
                                  int32 frameRadius,
                                  uint8 occupationSlot,
                                  TArray<AActor*>& outOccupants);

    int32 GetOccupantsInSectors(const FIntPoint& centralTile,
                                int32 areaRadius,
                                int32 firstSectorDirection,
                                int32 lastSectorDirection,
                                uint8 occupationSlot,
                                TArray<AActor*>& outOccupants);

#if WITH_HEX_TILE_GRID_DEBUG

    void DebugSetWorldContextObject(UObject* worldContextObject);

#endif // #if WITH_HEX_TILE_GRID_DEBUG

protected:

    static const float mRaycastStep;

    /**
        Tile data contains occupants and occupation slots.
    
        ~!~ ~R~ For the sake of optimization we can devide tile data into two data structures:
        1) Huge array containing each tile's occupation mask. Due to the minimal weight (1 byte) of this mask it would be relatevely small and quick.
        2) Map / Hash-map which would contain occupants for tiles. As we wouldn't have occupant for the most systems we don't need to spend so
        much memory for completely empty arrays of pointers. While searching for a path we use only occupation masks.
    */
    struct TileOccupants
    {
        TWeakObjectPtr<AActor> mContainer[tOccupantSlotsCount];
    };

    /**
        Object for path search process. Initialize and activated for processing search on thread.
    */
    friend class SearchTask;
    class SearchTask
    {
    public:
        explicit SearchTask(const HexTileGrid<tOccupantSlotsCount>& hexTileMap,
                            const FIntPoint& from,
                            const FIntPoint& to,
                            int32 occupationSlotsToBeFree,
                            int32 occupationSlotsInterruption,
                            uint32 outputLimit,
                            uint32 depthLimit,
                            bool ignoreTargetTileOccupation);
        SearchTask(const SearchTask&) = delete;
        SearchTask(SearchTask&&) = delete;
        TArray<FIntPoint> operator()();

    private:
        struct OpenTile
        {
            CompactTile mTile;
            CompactTile mParent;
            int32 mTotalCost = 0;
            uint32 mDepth = 0;
        };

        struct ClosedTile
        {
            CompactTile mTile;
            CompactTile mParent;
            int32 mTotalCost = 0;

            bool operator==(const ClosedTile& right) const
            {
                return mTile == right.mTile;
            }
        };

        static const int32 mToNeighbourWeight;

    private:
        const HexTileGrid<tOccupantSlotsCount>& mTileMap;
        FIntPoint mFrom;
        FIntPoint mTo;
        int32 mOccupationSlotsToBeFree;
        int32 mOccupationSlotsInterruption;
        uint32 mOutputLimit;
        uint32 mDepthLimit;
        TArray<OpenTile> mOpenList;
        TArray<ClosedTile> mClosedList;
        bool mIgnoreTargetTileOccupation = false;
    };

private:

    int32 mWidth = 0;

    int32 mHeight = 0;

    float mTileRadius = 100.0f;

    TArray<uint8> mTilesOccupations;

    TMap<FIntPoint, TileOccupants> mOccupants;

    FEventTileOccupation mEventTileOccupied;

    FEventTileOccupation mEventTileReleased;

#if WITH_HEX_TILE_GRID_DEBUG

    TWeakObjectPtr<UObject> mDebugWorldContextObject;

#endif // #if WITH_HEX_TILE_GRID_DEBUG

private:

    uint32 GetOccupationSlotIndex(uint8 occupationSlot) const;

    float GetTileHeight() const;

    float GetColumnWidth() const;

    FVector DetermineTilePosition(const FIntPoint& tile) const;

    /**
        NOTE: Do not use while looping mOccupants!
    */
    AActor* SetTileOccupant(const FIntPoint& tile,
                            uint8 occupationSlot,
                            AActor* pNewOccupant);
};

#include "HexTileGrid.impl.h"