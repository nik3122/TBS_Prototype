// Fill out your copyright notice in the Description page of Project Settings.

#include "Arena/Trap_Base.h"
#include "Arena/ArenaUtilities.h"
#include "Arena/BattleLog/BattleLog.h"
#include "Arena/BattleLog/BattleLogItem_CreatureTrapped.h"
#include "Arena/CreatureBase.h"
#include "Arena/Damageable.h"
#include "Arena/StatusEffectsManager.h"
#include "Arena/TrapSystem.h"


bool ATrap_Base::IsInterruptingMovement() const
{
    return mIsInterruptingMovement;
}

UTexture2D* ATrap_Base::GetTrapImage() const
{
    return mpTrapImage;
}

void ATrap_Base::SetupTrap(const FIntPoint& placementTile)
{
    mPlacementTile = placementTile;
    OnSetupTrap();
}

void ATrap_Base::SetCreatureTrapped(ACreatureBase* pTrappedCreature, int32 executionPriority)
{
    mpTrappedCreature = pTrappedCreature;
    mpTrappedCreature->GetTiledMovementComponent()->mEventTileReached.AddDynamic(this, &ATrap_Base::OnTrappedCreatureReachedTile);
    ApplyTrap();
}

void ATrap_Base::BeginPlay()
{
    Super::BeginPlay();

    UArenaUtilities::GetTrapSystem(this)->RegisterTrap(this);
    UE_LOG(ArenaCommonLog, VeryVerbose, TEXT("ATrap_Base::BeginPlay() NOT AN ERROR"));
}

void ATrap_Base::OnSetupTrap_Implementation()
{
    UE_LOG(ArenaCommonLog, Verbose, TEXT("Here the trap can play setup animation."));
}

void ATrap_Base::ApplyTrap_Implementation()
{
    {
        mDamageableTask.mPriority = mExecutionPriority;
        mDamageableTask.mpInitiator = this;
        UDamageable* pDamageable = mpTrappedCreature->GetDamageableComponent();
        pDamageable->ExecuteDamageableTask(mDamageableTask);
    }

    {
        UStatusEffectsManager* pStatusEffectsManage = mpTrappedCreature->GetStatusEffectsManagerComponent();
        for (FStatusEffectTask& task : mStatusEffectTasks)
        {
            task.mPriority = mExecutionPriority;
            pStatusEffectsManage->ExecuteStatusEffectTask(task);
        }
    }

    UBattleLog* pBattleLog = UArenaUtilities::GetBattleLog(this);
    pBattleLog->PushBattleLogItem(pBattleLog->CreateBattleLogItem_CreatureTrapped(this, mpTrappedCreature));
}

void ATrap_Base::OnTrappedCreatureReachedTile(UTiledMovement* pTiledMovement, const FIntPoint& tile)
{
    if (tile == mPlacementTile)
    {
        pTiledMovement->mEventTileReached.RemoveDynamic(this, &ATrap_Base::OnTrappedCreatureReachedTile);
        OnTriggered();
    }
}

void ATrap_Base::OnTriggered_Implementation()
{
    Destroy();
}