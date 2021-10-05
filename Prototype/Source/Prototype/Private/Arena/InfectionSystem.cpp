
#include "Arena/InfectionSystem.h"
#include "Arena/ArenaUtilities.h"
#include "Arena/ArenaUnit_Interface.h"
#include "Arena/StatusEffectsManager.h"
#include "Arena/StatusEffect_Infection.h"
#include "Arena/TiledMovement.h"
#include "Arena/PassMap.h"
#include "Arena/TurnsManager.h"
#include "EngineUtils.h"
#include "Niagara/Public/NiagaraComponent.h"
#include "Niagara/Public/NiagaraFunctionLibrary.h"


void AInfectionSystem::SetupSystem(APassMap* pPassMap)
{
    mInfectionMap.SetNum(pPassMap->GetWidth() * pPassMap->GetHeight());

    for (FInfectedTile& infectedTile : mInfectedTiles)
    {
        infectedTile.mIsSetManually = true;
        SpawnVisuals(infectedTile, pPassMap);
    }
}

void AInfectionSystem::RegisterInfectable(AStatusEffect_Infection* pStatusEffect)
{
    AActor* pTarget = pStatusEffect->GetOwner();
    check(IsValid(pTarget));
    if (pTarget->Implements<UArenaUnit_Interface>())
    {
        UTiledMovement* pTargetTiledMovement = IArenaUnit_Interface::Execute_GetTiledMovement(pTarget);
        if (IsValid(pTargetTiledMovement))
        {
            RegisterInfectableInner(pStatusEffect, pTargetTiledMovement);
        }
        else
        {
            UE_LOG(ArenaCommonLog, Warning, TEXT("AStatusEffect_Infection's owner %s is not creature."), *pTarget->GetName());
        }
    }
    else
    {
        UE_LOG(ArenaCommonLog, Fatal, TEXT("StatusEffect_Infection's owner must implement UArenaUnit_Interface."));
    }
}

void AInfectionSystem::UnregisterInfectable(AStatusEffect_Infection* pStatusEffect)
{
    mRegisteredInfectables.RemoveAllSwap([pStatusEffect](const FInfectableInfo& infectable) {
        return pStatusEffect == infectable.mpStatusEffect;
    });
}

void AInfectionSystem::RegisterInfectableInner(AStatusEffect_Infection* pStatusEffect, UTiledMovement* pPassMapMovement)
{
    FInfectableInfo info {};
    info.mpPassMapMovement = pPassMapMovement;
    info.mpStatusEffect = pStatusEffect;
    mRegisteredInfectables.AddUnique(info);
    if (UArenaUtilities::AreSystemsSetup(this))
    {
        //
    }
}

void AInfectionSystem::SetTileInfected(const FIntPoint& tile, int32 duration)
{
    bool removeTile = duration == 0;

    //  Normalize duration, increment 1 to compensate update at the end of the current turn:
    duration = duration < 0 ? TNumericLimits<decltype(FInfectedTile::mDuration)>::Max() : duration + 1;

    int32 elementIndex = mInfectedTiles.FindLastByPredicate([tile](const FInfectedTile& infectedTile) {
        return infectedTile.mTile == tile;
    });
    if (elementIndex != INDEX_NONE)
    {
        if (removeTile == false)
        {
            mInfectedTiles[elementIndex].mIsSetManually = true;
            mInfectedTiles[elementIndex].mDuration = duration;
        }
        //  Only manually set tiles can be manually removed:
        else if (mInfectedTiles[elementIndex].mIsSetManually &&
                mInfectedTiles[elementIndex].mIsGenerated == false)
        {
            mInfectedTiles[elementIndex].mpSpawnedNiagara->Deactivate();
            mInfectedTiles.RemoveAtSwap(elementIndex);
        }
    }
    else if (removeTile == false)
    {
        FInfectedTile infectedTile;
        infectedTile.mTile = tile;
        infectedTile.mDuration = duration;
        infectedTile.mIsSetManually = true;
        SpawnVisuals(infectedTile, nullptr);
        mInfectedTiles.Add(infectedTile);
    }
}

void AInfectionSystem::OnFinalizeTurn()
{
    //  Infection "power" determines how much infectables influance the tile.
    //  All uninfected get infected if they are at tiles with 1+ power.
    //  All infected continue their infection ONLY if they are at tiles with 2+ power - which means they are infected by OTHER infectables excluding itself.
    //  If an infected infectable placed at a tile infected only by itself - infection shouldn't grow.

    int32 currentTurn = UArenaUtilities::GetTurnsManager(this)->GetCurrentTurn();

    //  Determine new sources of infection (tiles under active infectables).
    //  Determine new set of infected tiles counting new sources.
    //  As we update on turns finalization - all final positions are known.
    //  Activate infection for all infectables in the new infected tiles.
    //  Remove all old infected tiles out of the new set.
    TArray<FIntPoint> infectionSources;
    for (const FInfectableInfo& infectable : mRegisteredInfectables)
    {
        //  Status effect may be unactive but be aplied at the same turn - thus we check both variants:
        if ((infectable.mpStatusEffect->IsActive() ||
            infectable.mpStatusEffect->GetLastTurnInfected() == currentTurn) &&
            infectable.mpStatusEffect->GetStatusEffectManager()->IsActive())
        {
            infectionSources.Push(infectable.mpPassMapMovement->GetCurrentTile());
        }
    }

    APassMap* pPassMap = UArenaUtilities::GetPassMap(this);

    //  Generate new infected tiles and update infection map:
    FMemory::Memzero(mInfectionMap.GetData(), mInfectionMap.Num() * sizeof(mInfectionMap[0]));
    TSet<FIntPoint> infectedTilesGen(infectionSources);
    for (const FIntPoint& sourceTile : infectionSources)
    {
        ++mInfectionMap[pPassMap->IndexFromCoord(sourceTile)];
        for (int32 i = 0; i < 6; ++i)
        {
            FIntPoint neighbor = pPassMap->GetNeighbour(sourceTile, i);
            if (pPassMap->IsTileValid(neighbor))
            {
                infectedTilesGen.Add(neighbor);
                ++mInfectionMap[pPassMap->IndexFromCoord(neighbor)];
            }
        }
    }

    mInfectedTiles.RemoveAllSwap([this, pPassMap, &infectedTilesGen](FInfectedTile& infectedTile) {
        infectedTile.mDuration = FMath::Max(0, infectedTile.mDuration - 1);
        infectedTile.mIsSetManually = infectedTile.mIsSetManually && infectedTile.mDuration > 0;
        //  Removing infected tiles which are already listed leaves only new tiles at the set:
        infectedTile.mIsGenerated = infectedTilesGen.Remove(infectedTile.mTile) != 0;
        if (infectedTile.mIsGenerated == false)
        {
            if (infectedTile.mIsSetManually)
            {
                ++mInfectionMap[pPassMap->IndexFromCoord(infectedTile.mTile)];
            }
            else
            {
                infectedTile.mpSpawnedNiagara->Deactivate();
                return true;
            }
        }
        return false;
    });

    //  Infect all infectables in the infected tiles:
    for (FInfectableInfo& infectable : mRegisteredInfectables)
    {
        UStatusEffectsManager* pStatusEffectManager = infectable.mpStatusEffect->GetStatusEffectManager();
        if (pStatusEffectManager->IsActive())
        {
            FIntPoint infectableTile = infectable.mpPassMapMovement->GetCurrentTile();
            //  ~!~ Check if the infectable is not the source of an infection:
            auto infectionPowerAtTile = mInfectionMap[pPassMap->IndexFromCoord(infectableTile)];
            if (infectionPowerAtTile > 0)
            {
                //  ~?~ No battle log item is needed?
                if (infectable.mpStatusEffect->IsActive() == false)
                {
                    FStatusEffectTask task = mFirstTimeInfectedStatusEffectTask;
                    task.mPriority = 0;
                    pStatusEffectManager->ExecuteStatusEffectTask(task);
                }
                //  If the infectable stands at the tile which was infected NOT only by him - proceed infection:
                else if (infectionPowerAtTile > 1)
                {
                    //  If status effect is already active - just proceed it:
                    FStatusEffectTask task = mFirstTimeInfectedStatusEffectTask;
                    task.mPriority = 0;
                    task.mDuration = 1;
                    task.mStage = infectable.mpStatusEffect->GetStage();
                    pStatusEffectManager->ExecuteStatusEffectTask(task);
                }
            }
        }
    }

    //  Add unique new infected tiles:
    for (const FIntPoint& tile : infectedTilesGen)
    {
        FInfectedTile infectedTile;
        infectedTile.mTile = tile;
        infectedTile.mIsGenerated = true;
        SpawnVisuals(infectedTile, pPassMap);
        mInfectedTiles.Push(infectedTile);
    }
}

#if WITH_EDITOR

void AInfectionSystem::PostEditChangeProperty(FPropertyChangedEvent& propertyChangedEvent)
{
    if (propertyChangedEvent.MemberProperty != nullptr &&
        propertyChangedEvent.MemberProperty->GetName() == "mInfectedTiles")
    {
        APassMap* pPassMap = *TActorIterator<APassMap>(GetWorld());
        for (FInfectedTile& infectedTile : mInfectedTiles)
        {
            infectedTile.mDuration = infectedTile.mDuration > 0 ? infectedTile.mDuration : TNumericLimits<decltype(FInfectedTile::mDuration)>::Max();
            if (pPassMap == nullptr || pPassMap->IsTileValid(infectedTile.mTile) == false)
            {
                infectedTile.mTile = { 0, 0 };
            }
        }
    }

    Super::PostEditChangeProperty(propertyChangedEvent);
}

#endif // #if WITH_EDITOR

void AInfectionSystem::SpawnVisuals(FInfectedTile& infectedTile, APassMap* pPassMap)
{
    FVector tilePosition = pPassMap->GetTilePositionLocal(infectedTile.mTile);
    infectedTile.mpSpawnedNiagara = UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(),
                                                                                   mpInfectedTilesNiagara,
                                                                                   tilePosition);
}
