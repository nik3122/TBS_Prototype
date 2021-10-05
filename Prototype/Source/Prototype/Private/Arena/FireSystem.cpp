// Fill out your copyright notice in the Description page of Project Settings.

#include "Arena/FireSystem.h"
#include "Arena/ArenaUtilities.h"
#include "Arena/ArenaManager.h"
#include "Arena/GameState_Arena.h"
#include "Arena/PassMap.h"
#include "Arena/TurnsManager.h"
#include "Arena/Damageable.h"
#include "Arena/TiledMovement.h"
#include "Arena/StatusEffectsManager.h"
#include "Arena/GameMode_ArenaFight.h"
#include "UnknownCommon.h"


AFireSystem::AFireSystem(const FObjectInitializer& objectInitializer)
    : Super(objectInitializer)
    , mpPassMap(nullptr)
    , mMapSize(0, 0)
{
    PrimaryActorTick.bCanEverTick = false;
}

void AFireSystem::SetupSystem(ATurnsManager* pTurnsManager, APassMap* pPassMap)
{
    pTurnsManager->mEventFinalizeTurn.AddDynamic(this, &AFireSystem::OnFinalizeTurn);

    mpPassMap = pPassMap;
    mpPassMap->mEventPassMapMovementRegistered.BindUObject(this, &AFireSystem::OnPassMapMovementRegistered);

    mMapSize.X = mpPassMap->GetHeight();
    mMapSize.Y = mpPassMap->GetWidth();
    mTilesFireData.SetNum(mpPassMap->GetTilesNumber());
}

void AFireSystem::ProcessMovement(const TArray<FIntPoint>& path, ACreatureBase* pMovingCreature)
{
    int32 trappedTimes = 0;
    for (const FIntPoint& tile : path)
    {
        trappedTimes += static_cast<int32>(IsTileOnFire(tile));
    }
    if (trappedTimes > 0)
    {
        UDamageable* pDamageable = pMovingCreature->GetDamageable();
        UStatusEffectsManager* pStatusEffectsManager = pMovingCreature->GetStatusEffectsManager();
        AffectTrappedInFire(trappedTimes, pDamageable, pStatusEffectsManager);
    }

    mProcessedMovements.Add(pMovingCreature->GetTiledMovement());
}

void AFireSystem::ProcessAllMovements()
{
    const TSet<UTiledMovement*> allTiledMovements = mpPassMap->GetRegisteredPassMapMovements();
    for (UTiledMovement* pTiledMovement : allTiledMovements)
    {
        if (mProcessedMovements.Contains(pTiledMovement) == false &&
            IsTileOnFire(pTiledMovement->GetCurrentTile()))
        {
            AActor* pOwner = pTiledMovement->GetOwner();
            if (pOwner->Implements<UArenaUnit_Interface>())
            {
                UDamageable* pDamageable = IArenaUnit_Interface::Execute_GetDamageable(pOwner);
                UStatusEffectsManager* pStatusEffectsManager = IArenaUnit_Interface::Execute_GetStatusEffectsManager(pOwner);
                AffectTrappedInFire(1, pDamageable, pStatusEffectsManager);
            }
        }
    }
}

void AFireSystem::UpdateFireDistribution()
{
    TArray<FIntPoint> tilesToProcess(mTilesToProcess);
    mTilesToProcess.SetNum(0, false);
    APassMap* pPassMap = GetPassMap();
    for (const FIntPoint& tile : tilesToProcess)
    {
        for (const FIntPoint& neighborTile : pPassMap->Neighbors(tile))
        {
            if (mpPassMap->IsTileValid(neighborTile) == false ||
                IsTileIgnitable(neighborTile) == false)
            {
                continue;
            }
            FTileFireData& neighborFireData = GetTileFireData(neighborTile);
            if (neighborFireData.mIsOnFire)
            {
                continue;
            }
            SetTileOnFire_Inner(neighborTile, neighborFireData);
        }
    }
}

bool AFireSystem::IsTileIgnitable(const FIntPoint& tile)
{
    return mpPassMap->IsTileFreeAtSlot(tile, EOccupationSlot::OccupationSlot_Fire) == false;
}

void AFireSystem::SetTileOnFire(const FIntPoint& tile)
{
    FTileFireData& tileFireData = GetTileFireData(tile);
    if (tileFireData.mIsOnFire || 
        IsTileIgnitable(tile) == false)
    {
        return;
    }
    SetTileOnFire_Inner(tile, tileFireData);
}

bool AFireSystem::IsTileOnFire(const FIntPoint& tile) const
{
    return GetTileFireData(tile).mIsOnFire;
}

void AFireSystem::ExtinguishFireOnTile(const FIntPoint& tile)
{
    FTileFireData& tileFireData = GetTileFireData(tile);
    if (tileFireData.mIsOnFire)
    {
        mTilesToProcess.RemoveSingleSwap(tile);
        return;
    }
    tileFireData.mIsOnFire = false;
}

void AFireSystem::BeginPlay()
{
    Super::BeginPlay();

    UArenaUtilities::GetArenaManager(this)->RegisterSystem(this);
}

void AFireSystem::OnFinalizeTurn()
{
    for (const auto& tile : mTilesOnFire)
    {
        UpdateTileOnFire(tile);
    }

    mProcessedMovements.Reset();
}

void AFireSystem::OnPassMapMovementRegistered(UTiledMovement* pPassMapMovement)
{
    pPassMapMovement->mEventTileReached.AddUniqueDynamic(this, &AFireSystem::OnTileReach);
}

void AFireSystem::OnTileReach(UTiledMovement* pTiledMovement, const FIntPoint& tile)
{
    if (IsTileOnFire(tile) == false)
    {
        return;
    }
    OnActorTrappedInFire(tile, pTiledMovement->GetOwner());
}

APassMap* AFireSystem::GetPassMap() const
{
    return mpPassMap;
}

void AFireSystem::SetTileOnFire_Inner(const FIntPoint& tile, FTileFireData& tileData)
{
    tileData.mIsOnFire = true;
    mTilesToProcess.AddUnique(tile);
    mTilesOnFire.Push(tile);

    NewTileOnFire(tile);

    mOnFireStarted.Broadcast(tile);
}

FTileFireData& AFireSystem::GetTileFireData(const FIntPoint& tile)
{
    int32 tileIndex = mpPassMap->IndexFromCoord(tile);
    return mTilesFireData[tileIndex];
}

const FTileFireData& AFireSystem::GetTileFireData(const FIntPoint& tile) const
{
    int32 tileIndex = mpPassMap->IndexFromCoord(tile);
    return mTilesFireData[tileIndex];
}

void AFireSystem::StoreState_Inner(ArenaStateContext& context, State_FireSystem& state) const
{
    state.mTilesOnFireCount = mTilesOnFire.Num();
    if (state.mTilesOnFireCount > 0)
    {
        //state.mpTilesOnFire = context.AllocMemory<FIntPoint>(state.mTilesOnFireCount);
        //FMemory::Memcpy(state.mpTilesOnFire, mTilesOnFire.GetData(), sizeof(FIntPoint) * state.mTilesOnFireCount);
        
        state.mpTilesOnFire = context.AllocMemory<int32>(state.mTilesOnFireCount);
        for (int32 i = 0; i < mTilesOnFire.Num(); ++i)
        {
            state.mpTilesOnFire[i] = GetPassMap()->IndexFromCoord(mTilesOnFire[i]);
        }
    }
    else
    {
        state.mpTilesOnFire = nullptr;
    }

    state.mTilesToProcessCount = mTilesToProcess.Num();
    if (state.mTilesToProcessCount > 0)
    {
        //state.mpTilesToProcess = context.AllocMemory<FIntPoint>(state.mTilesToProcessCount);
        //FMemory::Memcpy(state.mpTilesToProcess, mTilesToProcess.GetData(), sizeof(FIntPoint) * state.mTilesToProcessCount);

        state.mpTilesToProcess = context.AllocMemory<int32>(state.mTilesToProcessCount);
        for (int32 i = 0; i < mTilesToProcess.Num(); ++i)
        {
            state.mpTilesToProcess[i] = GetPassMap()->IndexFromCoord(mTilesToProcess[i]);
        }
    }
    else
    {
        state.mpTilesToProcess = nullptr;
    }
}

bool AFireSystem::RestoreState_Inner(const State_FireSystem& state)
{

    return true;
}

void AFireSystem::AffectTrappedInFire(int32 trappedTimes,
                                      UDamageable* pDamageable,
                                      UStatusEffectsManager* pStatusEffectsManager)
{
    if (IsValid(pDamageable))
    {
        FDamageableTask task = {};
        task.mValue = mDamageToDealOnTrapped * trappedTimes;
        task.mTaskType = EDamageableTaskType::Damage;
        //  ~TODO~ task.mPriority may be set from the action which call "StepAtTile".
        task.mpInitiator = this;
        pDamageable->ExecuteDamageableTask(task);
    }
    if (IsValid(pStatusEffectsManager))
    {
        pStatusEffectsManager->ExecuteStatusEffectTask(mStatusEffectTaskOnTrapped);
    }
}
