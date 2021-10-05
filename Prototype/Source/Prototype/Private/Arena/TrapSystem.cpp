

#include "Arena/TrapSystem.h"
#include "Arena/ArenaUtilities.h"
#include "Arena/ArenaManager.h"
#include "Arena/CreatureBase.h"
#include "Arena/ActionManager.h"
#include "Arena/TurnsManager.h"
#include "Arena/Trap_Base.h"
#include "Arena/GameState_Arena.h"
#include "EngineUtils.h"


void UTrapSystem::SetupSystem(APassMap* pPassMap)
{
    mpPassMap = pPassMap;
    UArenaUtilities::GetArenaManager(this)->mEventSystemsSetup.AddDynamic(this, &UTrapSystem::OnSystemsSetup);
}

void UTrapSystem::RegisterTrap(ATrap_Base* pTrap)
{
    mRegisteredTraps.Push(pTrap);

    if (UArenaUtilities::AreSystemsSetup(this))
    {
        SetupTrap(pTrap);
    }
}

void UTrapSystem::ProcessMovement(const TArray<FIntPoint>& path, ACreatureBase* pMovingCreature)
{
    for (const FIntPoint& tile : path)
    {
        if (ATrap_Base* pTrap = GetTrapAtTile(tile))
        {
            int32 executionPriority = pMovingCreature->GetActionManagerComponent()->GetExecutionPriority();
            pTrap->SetCreatureTrapped(pMovingCreature, executionPriority);
            mpPassMap->FreeTile(tile, EOccupationSlot::OccupationSlot_Trap);
            mpPassMap->FreeTile(tile, EOccupationSlot::OccupationSlot_MovementInterrupter);
            mTriggeredTraps.Push(pTrap);
        }
    }
}

ATrap_Base* UTrapSystem::TrySpawnTrap(TSubclassOf<ATrap_Base> trapClass, const FIntPoint& tile)
{
    check(mpPassMap->IsTileValid(tile));
    if (GetTrapAtTile(tile) != nullptr) return nullptr;
    FVector tilePosition = mpPassMap->GetTilePositionWorld(tile);
    ATrap_Base* pSpawnedTrap = GetWorld()->SpawnActor<ATrap_Base>(trapClass, tilePosition, FRotator::ZeroRotator);
    return pSpawnedTrap;
}

ATrap_Base* UTrapSystem::GetTrapAtTile(const FIntPoint& tile)
{
    return Cast<ATrap_Base>(mpPassMap->GetOccupant(tile, EOccupationSlot::OccupationSlot_Trap));
}

void UTrapSystem::RemoveTrapAtTile(const FIntPoint& tile)
{
    if (ATrap_Base* pTrap = GetTrapAtTile(tile))
    {
        mpPassMap->FreeTile(tile, EOccupationSlot::OccupationSlot_Trap);
        mpPassMap->FreeTile(tile, EOccupationSlot::OccupationSlot_MovementInterrupter);
        pTrap->Destroy();
    }
}

void UTrapSystem::OnSystemsSetup()
{
    //  Setup registered traps:
    for (ATrap_Base* pTrap : mRegisteredTraps)
    {
        SetupTrap(pTrap);
    }
    //  Trap may not be registered if it was spawned before TrapSystem (due to network lag or so). Register it now:
    for (TActorIterator<ATrap_Base> iterTrap(GetWorld()); iterTrap; ++iterTrap)
    {
        ATrap_Base* pTrap = *iterTrap;
        if (mRegisteredTraps.Contains(pTrap) == false)
        {
            RegisterTrap(pTrap);
        }
    }
    ATurnsManager* pTurnsManager = UArenaUtilities::GetTurnsManager(this);
    pTurnsManager->mEventFinalizeTurn.AddDynamic(this, &UTrapSystem::OnFinalizeTurn);
}

void UTrapSystem::OnFinalizeTurn()
{
    for (ATrap_Base* pTrap : mTriggeredTraps)
    {
        if (IsValid(pTrap))
        {
            pTrap->Destroy();
        }
    }
    mTriggeredTraps.Empty();
}

void UTrapSystem::SetupTrap(ATrap_Base* pTrap)
{
    FIntPoint tile = mpPassMap->GetTileInPositionWorld(pTrap->GetActorLocation());
    if (mpPassMap->IsTileFreeAtSlot_Safe(tile, EOccupationSlot::OccupationSlot_Trap))
    {
        mpPassMap->OccupyTile(tile, EOccupationSlot::OccupationSlot_Trap, pTrap);
        if (pTrap->IsInterruptingMovement())
        {
            mpPassMap->OccupyTile(tile, EOccupationSlot::OccupationSlot_MovementInterrupter, pTrap);
        }
        pTrap->SetupTrap(tile);
    }
    else
    {
        ATrap_Base* pTrapAlreadyInTile = Cast<ATrap_Base>(mpPassMap->GetOccupant(tile, EOccupationSlot::OccupationSlot_Trap));
        UE_LOG(ArenaCommonLog, Error, TEXT("Trying to place trap \"%s\" at [%i;%i], which already contains a trap \"%s\""), *pTrap->GetName(), tile.X, tile.Y, *pTrapAlreadyInTile->GetName());
    }
}
