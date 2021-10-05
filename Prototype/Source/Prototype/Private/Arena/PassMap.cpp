// Fill out your copyright notice in the Description page of Project Settings.

#include "Arena/PassMap.h"
#include "BitmaskOperator.h"
#include "UnknownCommon.h"
#include "Arena/ArenaUtilities.h"
#include "Arena/ArenaManager.h"
#include "Arena/GameState_Arena.h"
#include "Arena/TiledMovement.h"
#include "Arena/TurnsManager.h"
#include "Arena/GameMode_ArenaFight.h"
#include "Arena/TileOperator_Interface.h"
#include "Arena/PartyMemberSpawnPoint.h"
#include "Arena/TrapSystem.h"
#include "Arena/InfectionSystem.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "MathUtil.h"


APassMap::APassMap(const FObjectInitializer& objectInitializer)
    : Super(objectInitializer)
{
    PrimaryActorTick.bCanEverTick = false;

    mpTrapSystem = CreateDefaultSubobject<UTrapSystem>("TrapSystem");

    mHexTileGrid.SetDelegateTileOccupied(ArenaHexTileGrid::FEventTileOccupation::CreateUObject(this, &APassMap::OnTileOccupied));
    mHexTileGrid.SetDelegateTileOccupied(ArenaHexTileGrid::FEventTileOccupation::CreateUObject(this, &APassMap::OnTileReleased));

    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
    RootComponent->SetMobility(EComponentMobility::Static);

    mSimplifiedTraceComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SimplifiedTraceMesh"));
    mSimplifiedTraceComponent->SetupAttachment(RootComponent);
    mSimplifiedTraceComponent->SetVisibleFlag(false);
    mSimplifiedTraceComponent->SetMobility(EComponentMobility::Static);

    mTilesInstances = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("TileInstances"));
    mTilesInstances->SetupAttachment(RootComponent);
    mTilesInstances->SetMobility(EComponentMobility::Static);

    mFirePlatesInstances = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("FirePlates"));
    mFirePlatesInstances->SetupAttachment(RootComponent);
    mFirePlatesInstances->SetMobility(EComponentMobility::Static);
}

void APassMap::BeginPlay()
{
    Super::BeginPlay();

    UNKNOWN_NOT_ARENA_EDITOR_BEGIN(this)

    if (mHexTileGrid.GetWidth() != mWidth ||
        mHexTileGrid.GetHeight() != mHeight)
    {
        BuildTileGrid();
    }

    for (auto& tile : mSpawnTilesPlayerParty)
    {
        checkf(IsTileValid(tile), TEXT("Invalid PlayerParty spawn tile (%i, %i)."), tile.X, tile.Y);
    }

    for (auto& tile : mSpawnTilesAIParty)
    {
        checkf(IsTileValid(tile), TEXT("Invalid AIParty spawn tile (%i, %i)."), tile.X, tile.Y);
    }

    UNKNOWN_NOT_ARENA_EDITOR_END

    UArenaUtilities::GetArenaManager(this)->RegisterSystem(this);
}

void APassMap::SetupSystem(ATurnsManager* pTurnsManager)
{
    pTurnsManager->mEventFinalizeTurn.AddDynamic(this, &APassMap::OnFinalizeTurn);
    mpTrapSystem->SetupSystem(this);
}

void APassMap::RegisterPassMapMovement(UTiledMovement* pTiledMovement)
{
    mRegisteredPassMapMovements.Add(pTiledMovement);

    if (pTiledMovement->IsOccupationEnabled())
    {
        FIntPoint occupiedTile = pTiledMovement->GetCurrentTile();
        if (IsTileValid(occupiedTile))
        {
            OccupyTile(occupiedTile,
                       pTiledMovement->GetOccupationSlot(),
                       pTiledMovement->GetOwner());
        }
    }

    mEventPassMapMovementRegistered.Execute(pTiledMovement);
}

void APassMap::UnregisterPassMapMovement(UTiledMovement* pTiledMovement)
{
    int32 removedCount = mRegisteredPassMapMovements.Remove(pTiledMovement);
    if (removedCount == 1)
    {
        if (pTiledMovement->IsOccupationEnabled())
        {
            FIntPoint occupiedTile = pTiledMovement->GetCurrentTile();
            if (IsTileValid(occupiedTile))
            {
                FreeTile(occupiedTile, pTiledMovement->GetOccupationSlot());
            }
        }
    }
    else
    {
        UE_LOG(ArenaCommonLog, Error, TEXT("Trying to unregister not registered pass map movement of %s."), *pTiledMovement->GetOwner()->GetName());
    }
}

const TSet<UTiledMovement*>& APassMap::GetRegisteredPassMapMovements() const
{
    return mRegisteredPassMapMovements;
}

UTrapSystem* APassMap::GetTrapSystem() const
{
    return mpTrapSystem;
}

bool APassMap::LineTraceTile(const FVector& traceStart,
                             const FVector& traceEnd,
                             FIntPoint& outTile) const
{
    UPrimitiveComponent* traceComponent = GetTraceComponent();
    if (traceComponent != nullptr)
    {
        FHitResult hitResult;
        FCollisionQueryParams queryParams = FCollisionQueryParams::DefaultQueryParam;
        bool isHitFound = traceComponent->LineTraceComponent(hitResult,
                                                             traceStart,
                                                             traceEnd,
                                                             queryParams);
        if (isHitFound)
        {
            outTile = GetTileInPositionWorld(hitResult.Location);
            return IsTileValid(outTile);
        }
    }
    return false;
}

const FIntPoint* APassMap::GetNeighborShifts(int32 parity) const
{
    return mHexTileGrid.mNeighbourShifts[parity & 1];
}

void APassMap::GetSpawnTiles(TArray<FIntPoint>& spawnTilesPlayerParty, TArray<FIntPoint>& spawnTilesAIParty) const
{
    spawnTilesPlayerParty = mSpawnTilesPlayerParty;
    spawnTilesAIParty = mSpawnTilesAIParty;
}

APassMap::ArenaHexTileGrid::NeighborsContainer APassMap::Neighbors(const FIntPoint& tile) const
{
    return mHexTileGrid.Neighbors(tile);
}

UPassMapData* APassMap::GetPassMapData() const
{
    return mpData;
}

int32 APassMap::GetWidth() const
{
    return mHexTileGrid.GetWidth();
}

int32 APassMap::GetHeight() const
{
    return mHexTileGrid.GetHeight();
}

float APassMap::GetWidthPhysical() const
{
    return (float)(GetWidth() * 3 + 1) * 0.5f * mTileRadius;
}

float APassMap::GetHeightPhysical() const
{
    return ((float)GetHeight() + 0.5f) * mTileRadius * TMathUtilConstants<float>::Sqrt3;
}

int32 APassMap::GetTilesNumber() const
{
    return mHexTileGrid.GetTilesNumber();
}

float APassMap::GetTileRadius() const
{
    return mHexTileGrid.GetTileRadius();
}

FVector APassMap::GetTilePositionLocal(const FIntPoint& tile) const
{
    return mHexTileGrid.GetTilePosition(tile);
}

FVector APassMap::GetTilePositionWorld(const FIntPoint& tile) const
{
    return GetActorTransform().TransformPosition(mHexTileGrid.GetTilePosition(tile));
}

FIntPoint APassMap::GetTileInPositionLocal(const FVector& position) const
{
    return mHexTileGrid.GetTileInPosition(position);
}

FIntPoint APassMap::GetTileInPositionWorld(const FVector& position) const
{
    return mHexTileGrid.GetTileInPosition(GetActorTransform().InverseTransformPosition(position));
}

FIntPoint APassMap::CoordFromIndex(int32 index) const
{
    return mHexTileGrid.CoordFromIndex(index);
}

int32 APassMap::IndexFromCoord(const FIntPoint& coord) const
{
    return mHexTileGrid.IndexFromCoord(coord);
}

FIntPoint APassMap::InvalidTile() const
{
    return FIntPoint(-1, -1);
}

bool APassMap::IsTileValid(const FIntPoint& tile) const
{
    return mHexTileGrid.IsTileValid(tile);
}

bool APassMap::IsNeighbour(const FIntPoint& isTile,
                           const FIntPoint& ofTile) const
{
    return mHexTileGrid.IsNeighbour(isTile, ofTile);
}

FIntPoint APassMap::GetNeighbour(const FIntPoint& tile,
                                 int32 direction) const
{
    return mHexTileGrid.GetNeighbour(tile, direction);
}

int32 APassMap::GetNeighbourDirection(const FIntPoint& neighbourTile,
                                      const FIntPoint& targetTile) const
{
    return mHexTileGrid.GetNeighbourDirection(neighbourTile, targetTile);
}

int32 APassMap::GetDirectionFromVector(const FVector& vector) const
{
    return mHexTileGrid.GetDirectionFromVector(vector);
}

int32 APassMap::GetDirectionBetweenTiles(const FIntPoint& fromTile,
                                         const FIntPoint& toTile) const
{
    return mHexTileGrid.GetDirectionBetweenTiles(fromTile, toTile);
}

FRotator APassMap::GetDirectionRotation(int32 tileDirection) const
{
    tileDirection %= 6;
    int32 normDirection = tileDirection + (tileDirection >= 0 ? 0 : 6);
    float singleSectorAngle = 360.f / 6.f;
    return FRotator(0.f, normDirection * singleSectorAngle, 0.f);
}

bool APassMap::CheckNeighborhood(const FIntPoint& targetTile,
                                 const FIntPoint& neighbourTile,
                                 int32& direction) const
{
    return mHexTileGrid.CheckNeighborhood(targetTile, neighbourTile, direction);
}

bool APassMap::IsNeighbourDirectionValid(int32 direction) const
{
    return mHexTileGrid.IsNeighbourDirectionValid(direction);
}

int32 APassMap::GetDistance(const FIntPoint& tileA,
                            const FIntPoint& tileB) const
{
    return mHexTileGrid.GetDistance(tileA, tileB);
}

bool APassMap::IsTileFree(const FIntPoint& tile,
                          int32 occupationSlotsToBeFree,
                          bool atLeastOne) const
{
    return mHexTileGrid.IsTileFree(tile, occupationSlotsToBeFree, atLeastOne);
}

bool APassMap::IsTileFree_Safe(const FIntPoint& tile,
                               int32 occupationSlotsToBeFree,
                               bool atLeastOne) const
{
    return mHexTileGrid.IsTileFree_Safe(tile, occupationSlotsToBeFree, atLeastOne);
}

void APassMap::OccupyTile(const FIntPoint& tile,
                          EOccupationSlot occupationSlot,
                          AActor* pOccupant)
{
    mHexTileGrid.OccupyTile(tile, static_cast<uint8>(occupationSlot), pOccupant);
}

void APassMap::OccupyTile_Safe(const FIntPoint& tile,
                               EOccupationSlot occupationSlot,
                               AActor* pOccupant)
{
    mHexTileGrid.OccupyTile_Safe(tile, static_cast<uint8>(occupationSlot), pOccupant);
}

AActor* APassMap::FreeTile(const FIntPoint& tile,
                           EOccupationSlot occupationSlot)
{
    return mHexTileGrid.FreeTile(tile, static_cast<uint8>(occupationSlot));
}

AActor* APassMap::FreeTile_Safe(const FIntPoint& tile,
                                EOccupationSlot occupationSlot)
{
    return mHexTileGrid.FreeTile_Safe(tile, static_cast<uint8>(occupationSlot));
}

void APassMap::ResetAllOccupationsAtSlot(EOccupationSlot occupationSlot,
                                         bool skipNotifications)
{
    mHexTileGrid.ResetAllOccupations(static_cast<uint8>(occupationSlot),
                                     skipNotifications);
}

bool APassMap::IsTileFreeAtSlot(const FIntPoint& tile,
                                EOccupationSlot occupationSlot) const
{
    return mHexTileGrid.IsTileFreeAtSlot(tile, static_cast<uint8>(occupationSlot));
}

bool APassMap::IsTileFreeAtSlot_Safe(const FIntPoint& tile,
                                     EOccupationSlot occupationSlot) const
{
    return mHexTileGrid.IsTileFreeAtSlot_Safe(tile, static_cast<uint8>(occupationSlot));
}

AActor* APassMap::GetOccupant(const FIntPoint& tile,
                              EOccupationSlot occupationSlot) const
{
    return mHexTileGrid.GetOccupant(tile, static_cast<uint8>(occupationSlot));
}

AActor* APassMap::GetOccupant_Safe(const FIntPoint& tile,
                                   EOccupationSlot occupationSlot) const
{
    return mHexTileGrid.GetOccupant_Safe(tile, static_cast<uint8>(occupationSlot));
}

TArray<FIntPoint> APassMap::FindPath(const FIntPoint& from,
                                     const FIntPoint& to,
                                     int32 occupationSlotsToBeFree,
                                     int32 occupationSlotsInterruption,
                                     int32 depthLimit,
                                     int32 outputLimit,
                                     bool ignoreTargetTileOccupation)
{
    return mHexTileGrid.FindPath(from, to,
                                 occupationSlotsToBeFree,
                                 occupationSlotsInterruption,
                                 depthLimit, outputLimit,
                                 ignoreTargetTileOccupation);
}

bool APassMap::DirectVisibility(const FIntPoint& from,
                                const FIntPoint& to,
                                int32 occupationSlotsToBeFree,
                                FIntPoint& outBlockingTile) const
{
    if (IsTileValid(from) == false ||
        IsTileValid(to) == false)
    {
        UE_LOG(ArenaCommonLog, Warning, TEXT("Trying to check direct visibility for invalid tiles: from [%i; %i] to [%i; %i]."),
               from.X,
               from.Y,
               to.X,
               to.Y);
        return false;
    }

    if (IsNeighbour(from, to))
    {
        return true;
    }

    return mHexTileGrid.DirectVisibility(from,
                                         to,
                                         occupationSlotsToBeFree,
                                         50.f,
                                         outBlockingTile);
}

int32 APassMap::GetLine(const FIntPoint& from,
                        const FIntPoint& to,
                        TArray<FIntPoint>& outTilesOfLine,
                        float raycastStep)
{
    return mHexTileGrid.GetLine(from, to,
                                outTilesOfLine,
                                raycastStep);
}

int32 APassMap::GetReachableTiles(const FIntPoint& centralTile,
                                  int32 depthLimit,
                                  int32 occupationSlotsToBeFree,
                                  TArray<FIntPoint>& outReachableTiles)
{
    return mHexTileGrid.GetReachableTiles(centralTile,
                                          depthLimit,
                                          occupationSlotsToBeFree,
                                          outReachableTiles);
}

int32 APassMap::GetHexaArea(const FIntPoint& centralTile,
                            int32 areaRadius,
                            TArray<FIntPoint>& outAreaTiles)
{
    return mHexTileGrid.GetHexaArea(centralTile,
                                    areaRadius,
                                    outAreaTiles);
}

int32 APassMap::GetHexaFrame(const FIntPoint& centralTile,
                             int32 frameRadius,
                             TArray<FIntPoint>& outAreaTiles)
{
    return mHexTileGrid.GetHexaFrame(centralTile,
                                     frameRadius,
                                     outAreaTiles);
}

int32 APassMap::GetHexaSectors(const FIntPoint& centralTile,
                               int32 sectorRadius,
                               int32 firstSectorDirection,
                               int32 lastSectorDirection,
                               TArray<FIntPoint>& outAreaTiles)
{
    return mHexTileGrid.GetHexaSectors(centralTile,
                                       sectorRadius,
                                       firstSectorDirection,
                                       lastSectorDirection,
                                       outAreaTiles);
}

int32 APassMap::GetHexaRing(const FIntPoint& centralTile,
                            int32 ringRadiusMin,
                            int32 ringRadiusMax,
                            TArray<FIntPoint>& outRingTiles)
{
    return mHexTileGrid.GetHexaRing(centralTile,
                                    ringRadiusMin,
                                    ringRadiusMax,
                                    outRingTiles);
}

void APassMap::ForEachTileInHexaArea(const FIntPoint& centralTile,
                                     int32 frameRadius,
                                     TScriptInterface<ITileOperator_Interface> tileOperator,
                                     bool includingCentralTile)
{
    auto OperateTile = [this, tileOperator](const FIntPoint& tile) { 
        return ITileOperator_Interface::Execute_OperateTile(tileOperator.GetObject(), this, tile);
    };
    mHexTileGrid.ForEachTileInHexaArea(centralTile,
                                       frameRadius,
                                       OperateTile,
                                       includingCentralTile);
}

void APassMap::ForEachTileInHexaFrame(const FIntPoint& centralTile,
                                      int32 frameRadius,
                                      TScriptInterface<ITileOperator_Interface> tileOperator)
{
    auto OperateTile = [this, tileOperator](const FIntPoint& tile) { 
        return ITileOperator_Interface::Execute_OperateTile(tileOperator.GetObject(), this, tile);
    };
    mHexTileGrid.ForEachTileInHexaFrame(centralTile,
                                        frameRadius,
                                        OperateTile);
}

int32 APassMap::GetRandomFreeTilesInHexaArea(const FIntPoint& centralTile,
                                             int32 areaRadius,
                                             int32 maxFreeTilesCount,
                                             int32 occupationSlotsToBeFree,
                                             TArray<FIntPoint>& outRandomFreeTiles)
{
    return mHexTileGrid.GetRandomFreeTilesInHexaArea(centralTile,
                                                     areaRadius,
                                                     maxFreeTilesCount,
                                                     occupationSlotsToBeFree,
                                                     outRandomFreeTiles);
}

void APassMap::ForEachTileInSectors(const FIntPoint& centralTile,
                                    int32 areaRadius,
                                    int32 firstSectorDirection,
                                    int32 lastSectorDirection,
                                    TScriptInterface<ITileOperator_Interface> tileOperator,
                                    bool includingCentralTile)
{
    auto OperateTile = [this, tileOperator](const FIntPoint& tile) { 
        return ITileOperator_Interface::Execute_OperateTile(tileOperator.GetObject(), this, tile);
    };
    mHexTileGrid.ForEachTileInSectors(centralTile,
                                      areaRadius,
                                      firstSectorDirection,
                                      lastSectorDirection,
                                      OperateTile,
                                      includingCentralTile);
}

void APassMap::GetRandomTilesChain(const FIntPoint& chainStartTile,
                                   int32 chainLength,
                                   const TArray<FIntPoint>& tilesToIgnore,
                                   TArray<FIntPoint>& outChainTiles)
{
    mHexTileGrid.GetRandomTilesChain(chainStartTile,
                                     chainLength,
                                     tilesToIgnore,
                                     outChainTiles);
}

int32 APassMap::GetOccupantsInHexaArea(const FIntPoint& centralTile,
                                       int32 areaRadius,
                                       EOccupationSlot occupationSlot,
                                       TArray<AActor*>& outOccupants)
{
    return mHexTileGrid.GetOccupantsInHexaArea(centralTile,
                                               areaRadius,
                                               static_cast<uint8>(occupationSlot),
                                               outOccupants);
}

int32 APassMap::GetOccupantsInHexaFrame(const FIntPoint& centralTile,
                                        int32 frameRadius,
                                        EOccupationSlot occupationSlot,
                                        TArray<AActor*>& outOccupants)
{
    return mHexTileGrid.GetOccupantsInHexaFrame(centralTile,
                                                frameRadius,
                                                static_cast<uint8>(occupationSlot),
                                                outOccupants);
}

int32 APassMap::GetOccupantsInSectors(const FIntPoint& centralTile,
                                      int32 areaRadius,
                                      int32 firstSectorDirection,
                                      int32 lastSectorDirection,
                                      EOccupationSlot occupationSlot,
                                      TArray<AActor*>& outOccupants)
{
    return mHexTileGrid.GetOccupantsInSectors(centralTile,
                                              areaRadius,
                                              firstSectorDirection,
                                              lastSectorDirection,
                                              static_cast<uint8>(occupationSlot),
                                              outOccupants);
}

void APassMap::SortCreaturesByDistanceToTile(TArray<ACreatureBase*>& creatures, FIntPoint tile, TArray<int32>& distances)
{
    using PairCreatureDistance = TPair<ACreatureBase*, int32>;
    TArray<PairCreatureDistance> creaturesDistances;
    creaturesDistances.Reserve(creatures.Num());
    for (int32 i = 0; i < creatures.Num(); ++i)
    {
        ACreatureBase* pCreature = creatures[i];
        int32 distance = GetDistance(tile, pCreature->GetTiledMovement()->GetCurrentTile());
        creaturesDistances.Emplace(PairCreatureDistance(pCreature, distance));
    }

    creaturesDistances.Sort([](const PairCreatureDistance& left, const PairCreatureDistance& right){
        return left.Value < right.Value;
    });

    distances.SetNum(creatures.Num());
    for (int32 i = 0; i < creatures.Num(); ++i)
    {
        creatures[i] = creaturesDistances[i].Key;
        distances[i] = creaturesDistances[i].Value;
    }
}

#if WITH_EDITOR

void APassMap::PostEditChangeProperty(FPropertyChangedEvent& propertyChangedEvent)
{
    if (propertyChangedEvent.GetPropertyName() == GET_MEMBER_NAME_CHECKED(APassMap, mTileRadius))
    {
        RebuildVisuals();
        K2_InitTileGrid();
    }
    else if (propertyChangedEvent.GetPropertyName() == GET_MEMBER_NAME_CHECKED(APassMap, mWidth))
    {
        if (mpData != nullptr)
        {
            mWidth = mpData->mWidth;
        }
        else if (mHexTileGrid.GetWidth() != mWidth)
        {
            BuildTileGrid();
            //mHexTileGrid.ResetSize(mWidth, mHexTileGrid.GetHeight(), true);
        }
    }
    else if (propertyChangedEvent.GetPropertyName() == GET_MEMBER_NAME_CHECKED(APassMap, mHeight))
    {
        if (mpData != nullptr)
        {
            mHeight = mpData->mHeight;
        }
        else if (mHexTileGrid.GetHeight() != mHeight)
        {
            BuildTileGrid();
            //mHexTileGrid.ResetSize(mHexTileGrid.GetWidth(), mHeight, true);
        }
    }
    else if (propertyChangedEvent.GetPropertyName() == GET_MEMBER_NAME_CHECKED(APassMap, mpData))
    {
        if (mpData != nullptr)
        {
            mWidth = mpData->mWidth;
            mHeight = mpData->mHeight;
        }
    }

    Super::PostEditChangeProperty(propertyChangedEvent);
}

void APassMap::SetTileMapData(UPassMapData* passMapData)
{
    mpData = passMapData;
    OnPassMapDataUpdated();
}

void APassMap::OnTileDataModified(const FIntPoint& modifiedTile)
{
    if (mpData == nullptr ||
        IsTileValid(modifiedTile) == false)
    {
        return;
    }

    FTileParameters* tileParams = mpData->GetTileParameters(modifiedTile);
    if (tileParams == nullptr)
    {
        return;
    }

    SetTileData(modifiedTile, *tileParams);

    RebuildVisuals(modifiedTile);
}

void APassMap::OnPassMapDataUpdated()
{
    BuildTileGrid();
}

#endif // #if WITH_EDITOR

void APassMap::BuildTileGrid()
{
    mHexTileGrid.SetTileRadius(mTileRadius);
    if (mpData == nullptr)
    {
        mHexTileGrid.ResetSize(mWidth, mHeight, true);

        //  Create and initialize an array of tile datas.
        //  The array contains a sequence of columns.
        for (int32 i = 0; i < mWidth * mHeight; ++i)
        {
            //  (T) Temporary randomize static occupation:
            FIntPoint tile = CoordFromIndex(i);
            if ((tile.X > 3 || tile.Y > 3) &&
                (FMath::Rand() % 5) > 2)
            {
                OccupyTile_Safe(tile, EOccupationSlot::OccupationSlot_Static, nullptr);
            }
        }
    }
    else
    {
        mWidth = mpData->mWidth;
        mHeight = mpData->mHeight;
        mHexTileGrid.ResetSize(mWidth, mHeight, true);

        //	Create and initialize an array of tile datas.
        //	The array contains a sequence of columns.
        for (int32 i = 0; i < mWidth * mHeight; ++i)
        {
            FIntPoint tile = CoordFromIndex(i);
            const FTileParameters* tileParameters = mpData->GetTileParameters(tile);
            if (tileParameters != nullptr)
            {
                if (tileParameters->mIsStaticlyOccupied)
                {
                    OccupyTile_Safe(tile, EOccupationSlot::OccupationSlot_Static, nullptr);
                }
                if (tileParameters->IsStaticLowObstacle())
                {
                    OccupyTile_Safe(tile, EOccupationSlot::OccupationSlot_StaticLowObstacle, nullptr);
                }
                if (tileParameters->IsStaticHighObstacle())
                {
                    OccupyTile_Safe(tile, EOccupationSlot::OccupationSlot_StaticHighObstacle, nullptr);
                }
                if (tileParameters->IsIgnitable())
                {
                    //  ~!~ Burnable tiles occupants must 
                    OccupyTile_Safe(tile, EOccupationSlot::OccupationSlot_Fire, nullptr);
                }
            }
        }
    }

    mSpawnTilesPlayerParty.Empty(5);
    mSpawnTilesAIParty.Empty(5);
    TInlineComponentArray<UPartyMemberSpawnPoint*, 20> spawnPoints(this);
    for (const UPartyMemberSpawnPoint* pSpawnPoint : spawnPoints)
    {
        if (pSpawnPoint->mParty == EParty::Party_Player)
        {
            mSpawnTilesPlayerParty.Push(GetTileInPositionLocal(pSpawnPoint->GetRelativeLocation()));
        }
        else if (pSpawnPoint->mParty == EParty::Party_AI)
        {
            mSpawnTilesAIParty.Push(GetTileInPositionLocal(pSpawnPoint->GetRelativeLocation()));
        }
    }

    RebuildVisuals();

    FVector traceComponentScale(GetHeightPhysical() / 100.f, GetWidthPhysical() / 100.f, 1.f);
    mSimplifiedTraceComponent->SetRelativeScale3D(traceComponentScale);

    K2_InitTileGrid();
}

void APassMap::OnFinalizeTurn()
{
    //  ~?~ IS IT OK TO DO IT ON FinalizeTurn?
    //	~!~ ~R~ Hard reset of arena unit occupations:

    //uint8 invDynamicOccupationMask = ~MakeBitMask(EOccupationSlot::OccupationSlot_ArenaUnit);
    //for (auto& tileOccupations : mTilesOccupations)
    //{
    //    tileOccupations &= invDynamicOccupationMask;
    //}
    //
    //uint32 dynamicOccupationSlotIndex = GetOccupationSlotIndex(static_cast<uint8>(EOccupationSlot::OccupationSlot_ArenaUnit));
    //TArray<FIntPoint> tilesWithEmptyOccupantsContainer;
    //for (auto& tileOccupants : mOccupants)
    //{
    //    tileOccupants.Value.mContainer[static_cast<int32>(EOccupationSlot::OccupationSlot_ArenaUnit)] = nullptr;
    //
    //    //	Check if there is at least one occupant at this tile:
    //    bool shouldBeRemoved = true;
    //    for (TWeakObjectPtr<AActor>& pOccupant : tileOccupants.Value.mContainer)
    //    {
    //        if (pOccupant.IsValid())
    //        {
    //            shouldBeRemoved = false;
    //            break;
    //        }
    //    }
    //    if (shouldBeRemoved)
    //    {
    //        tilesWithEmptyOccupantsContainer.Push(tileOccupants.Key);
    //    }
    //}
    //for (const FIntPoint& tileToRemove : tilesWithEmptyOccupantsContainer)
    //{
    //    mOccupants.Remove(tileToRemove);
    //}

    mHexTileGrid.ResetAllOccupations(static_cast<int32>(EOccupationSlot::OccupationSlot_ArenaUnit), true);

    for (UTiledMovement* pPassMapMovement : mRegisteredPassMapMovements)
    {
        if (pPassMapMovement->IsOccupationEnabled())
        {
            OccupyTile(pPassMapMovement->GetCurrentTile(),
                       pPassMapMovement->GetOccupationSlot(),
                       pPassMapMovement->GetOwner());
        }
    }

    UArenaUtilities::GetInfectionSystem(this)->OnFinalizeTurn();
}

void APassMap::OnConstruction(const FTransform& transform)
{
    Super::OnConstruction(transform);

#if WITH_HEX_TILE_GRID_DEBUG

    mHexTileGrid.DebugSetWorldContextObject(this);

#endif // #if WITH_HEX_TILE_GRID_DEBUG

    BuildTileGrid();
}

void APassMap::StoreState_Inner(ArenaStateContext& context, State_PassMap& state) const
{
    state.mSizeX = mHeight;
    state.mSizeY = mWidth;
    int32 tilesCount = state.mSizeX * state.mSizeY;
    const TArray<uint8>& occupationData = mHexTileGrid.GetOccupationData();
    check(tilesCount == occupationData.Num() && "Number of tiles occupations should equal the total number of tiles.");
    //TArray<State_PassMap::OccupiedTile> occupiedTiles = 
    /*state.mpOccupiedTiles = context.AllocMemory<State_PassMap::OccupiedTile>(tilesCount);
    state.mOccupiedTilesCount = 0;
    for (int32 i = 0; i < tilesCount; ++i)
    {
    if (mTilesOccupations[i] != 0)
    {
    state.mpOccupiedTiles[i].mTileIndex = i;
    state.mpOccupiedTiles[i].mOccupationMask = mTilesOccupations[i];
    ++state.mOccupiedTilesCount;
    }
    }*/
    state.mpTilesOccupations = context.AllocMemory<uint8>(tilesCount);
    FMemory::Memcpy(state.mpTilesOccupations, occupationData.GetData(), tilesCount);
}

bool APassMap::RestoreState_Inner(const State_PassMap& state)
{
    return true;
}

void APassMap::OnTileOccupied(const FIntPoint& tile, uint8 occupationSlot, AActor* pOccupant)
{
    mEventTileOccupied.Broadcast(tile, static_cast<EOccupationSlot>(occupationSlot), pOccupant);
}

void APassMap::OnTileReleased(const FIntPoint& tile, uint8 occupationSlot, AActor* pOccupant)
{
    mEventTileReleased.Broadcast(tile, static_cast<EOccupationSlot>(occupationSlot), pOccupant);
}

UPrimitiveComponent* APassMap::GetTraceComponent() const
{
    return mSimplifiedTraceComponent;
}

void APassMap::SetTileData(const FIntPoint& tile,
                           const FTileParameters& tileData)
{
    if (tileData.IsStaticLowObstacle())
    {
        OccupyTile(tile, EOccupationSlot::OccupationSlot_StaticLowObstacle, nullptr);
    }
    else
    {
        FreeTile(tile, EOccupationSlot::OccupationSlot_StaticLowObstacle);
    }

    if (tileData.IsStaticHighObstacle())
    {
        OccupyTile(tile, EOccupationSlot::OccupationSlot_StaticHighObstacle, nullptr);
    }
    else
    {
        FreeTile(tile, EOccupationSlot::OccupationSlot_StaticHighObstacle);
    }

    if (tileData.IsIgnitable())
    {
        OccupyTile(tile, EOccupationSlot::OccupationSlot_Fire, nullptr);
    }
    else
    {
        FreeTile(tile, EOccupationSlot::OccupationSlot_Fire);
    }
}

void APassMap::RebuildVisuals()
{
    if (mWidth > 0 &&
        mHeight > 0)
    {
        const int32 oldInstancesCount = mTilesInstances->GetInstanceCount();
        const int32 newInstancesCount = mWidth * mHeight;

        TArray<FTransform> tileTransforms;
        TArray<FTransform> newTileTransforms;

        TArray<FTransform> firePlateTransforms;
        TArray<FTransform> newFirePlateTransforms;

        tileTransforms.SetNumUninitialized(oldInstancesCount);
        firePlateTransforms.SetNumUninitialized(oldInstancesCount);

        if (newInstancesCount > oldInstancesCount)
        {
            newTileTransforms.SetNumUninitialized(newInstancesCount - oldInstancesCount);
            newFirePlateTransforms.SetNumUninitialized(newInstancesCount - oldInstancesCount);
        }

        for (int32 i = 0; i < FMath::Max(oldInstancesCount, newInstancesCount); ++i)
        {
            if (i < newInstancesCount)
            {
                FIntPoint tile = CoordFromIndex(i);
                if (i >= oldInstancesCount)
                {
                    GetTileTransformLocal(tile,
                                          newTileTransforms[i - oldInstancesCount],
                                          newFirePlateTransforms[i - oldInstancesCount]);
                }
                else
                {
                    GetTileTransformLocal(tile,
                                          tileTransforms[i],
                                          firePlateTransforms[i]);
                }
            }
            else
            {
                tileTransforms[i].SetLocation(FVector::ZeroVector);
                tileTransforms[i].SetRotation(FQuat::Identity);
                tileTransforms[i].SetScale3D(FVector::ZeroVector);
                firePlateTransforms[i] = tileTransforms[i];
            }
        }

        mTilesInstances->BatchUpdateInstancesTransforms(0, tileTransforms, false, true, true);
        if (newTileTransforms.Num() > 0)
        {
            mTilesInstances->AddInstances(newTileTransforms, false);
        }

        mFirePlatesInstances->BatchUpdateInstancesTransforms(0, firePlateTransforms, false, true, true);
        if (newFirePlateTransforms.Num() > 0)
        {
            mFirePlatesInstances->AddInstances(newFirePlateTransforms, false);
        }
    }
    else
    {
        UE_LOG(ArenaCommonLog, Warning, TEXT("APassMap \"%s\" can't rebuild visuals due to invalid size [%i; %i]."), 
               *GetName(),
               mHeight,
               mWidth);
    }
    return;

    if (mWidth > 0 && mHeight > 0)
    {
        const int32 tilesCount = mWidth * mHeight;
        const int32 instancesCount = mTilesInstances->GetInstanceCount();

        TArray<FTransform> oldtilesTransforms;
        oldtilesTransforms.SetNum(FMath::Min(tilesCount, instancesCount));

        TArray<FTransform> newTilesTransforms;
        newTilesTransforms.SetNum(FMath::Max(0, tilesCount - instancesCount));

        for (int32 x = 0; x < mHeight; ++x)
        {
            for (int32 y = 0; y < mWidth; ++y)
            {
                FIntPoint tile = { x, y };
                int32 tileIndex = IndexFromCoord(tile);
                FTransform tileTransform;
                FTransform firePlateTransform;
                GetTileTransformLocal(tile,
                                      tileTransform,
                                      firePlateTransform);
                if (tileIndex < mTilesInstances->GetInstanceCount())
                {
                    oldtilesTransforms[tileIndex] = tileTransform;
                }
                else
                {
                    newTilesTransforms[tileIndex - oldtilesTransforms.Num()] = tileTransform;
                }
            }
        }

        mTilesInstances->BatchUpdateInstancesTransforms(0, oldtilesTransforms,
                                                       false, true, true);

        const int32 instancesToHideCount = FMath::Max(0, instancesCount - tilesCount);
        if (newTilesTransforms.Num() > 0)
        {
            mTilesInstances->AddInstances(newTilesTransforms, false);
        }
        else if (instancesToHideCount > 0)
        {
            //  ~?~ Remove out of range instances.
            TArray<FTransform> tilesToHideTransforms;
            FTransform zeroScaleTransform = FTransform::Identity;
            zeroScaleTransform .SetScale3D(FVector::ZeroVector);
            tilesToHideTransforms.SetNum(instancesToHideCount, false);
            //  This is the correct initialization of the zero-scale transformations.
            for (FTransform& transform : tilesToHideTransforms)
            {
                transform = zeroScaleTransform;
            }
            mTilesInstances->BatchUpdateInstancesTransforms(tilesCount, tilesToHideTransforms,
                                                            false, true, true);
        }
    }
    else
    {
        UE_LOG(ArenaCommonLog, Warning, TEXT("APassMap \"%s\" can't rebuild visuals due to invalid size [%i; %i]."), 
               *GetName(),
               mHeight,
               mWidth);
    }
}

void APassMap::RebuildVisuals(const FIntPoint& modifiedTile)
{
    if (IsTileValid(modifiedTile) == false)
    {
        return;
    }

    int32 tileIndex = IndexFromCoord(modifiedTile);
    if (mTilesInstances->GetInstanceCount() <= tileIndex)
    {
        return;
    }

    FTransform tileTransform = FTransform::Identity;
    FTransform firePlateTransform = FTransform::Identity;
    GetTileTransformLocal(modifiedTile,
                          tileTransform,
                          firePlateTransform);
    mTilesInstances->UpdateInstanceTransform(tileIndex,
                                             tileTransform,
                                             false, true, true);
    mFirePlatesInstances->UpdateInstanceTransform(tileIndex,
                                                  firePlateTransform,
                                                  false, true, true);
}

void APassMap::GetTileTransformLocal(const FIntPoint& tile,
                                     FTransform& outTileTransform,
                                     FTransform& outFirePlateTransform)
{
    bool isHighObstacle = !IsTileFreeAtSlot(tile, EOccupationSlot::OccupationSlot_StaticHighObstacle);
    bool isLowObstacle = isHighObstacle ? false : !IsTileFreeAtSlot(tile, EOccupationSlot::OccupationSlot_StaticLowObstacle);
    bool isIgnitable = !IsTileFreeAtSlot(tile, EOccupationSlot::OccupationSlot_Fire);

    isHighObstacle |= !IsTileFreeAtSlot(tile, EOccupationSlot::OccupationSlot_Static);

    outTileTransform = FTransform::Identity;
    outTileTransform.SetLocation(GetTilePositionLocal(tile));
    outFirePlateTransform = outTileTransform;

    FVector tileScale(1.f);
    tileScale.X = tileScale.Y = (isHighObstacle ? 0.2f : (isLowObstacle ? 0.5f : 1.0f)) * mTileRadius / sUnitScaleRadius;
    outTileTransform.SetScale3D(tileScale);

    FVector firePlateScale(1.f);
    firePlateScale.X = firePlateScale.Y = isIgnitable ? tileScale.X : 0.f;
    outFirePlateTransform.SetScale3D(firePlateScale);
}
