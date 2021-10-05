

#include "Arena/Actions/Action_SeparateMovement_Attack.h"
#include "Arena/ActionManager.h"
#include "Arena/AnimationManager.h"
#include "Arena/ArenaUnitAttributes.h"
#include "Arena/ArenaUtilities.h"
#include "Arena/BattleLog/BattleLog.h"
#include "Arena/BattleLog/BattleLogItem_DamageCreature.h"
#include "Arena/CreatureBase.h"
#include "Arena/PassMap.h"
#include "Arena/TiledMovement.h"
#include "Arena/WeaponOperator.h"
#include "Kismet/KismetSystemLibrary.h"
#include "TileMaps/HexTilePattern.h"


AAction_SeparateMovement_Attack::AAction_SeparateMovement_Attack(const FObjectInitializer& initializer)
    : Super(initializer)
{
    PrimaryActorTick.bCanEverTick = true;
}

void AAction_SeparateMovement_Attack::Tick(float deltaSeconds)
{
    Super::Tick(deltaSeconds);

    if (IsSetup() == false)
    {
        return;
    }

    if (mIsExecuting)
    {
        APassMap* passMap = GetExecutorTiledMovement()->GetPassMap();
        FVector executorLocation = GetExecutorActor()->GetActorLocation();
        UKismetSystemLibrary::DrawDebugCone(this,
                                            executorLocation + FVector(0.f, 0.f, 200.f),
                                            FVector(0.f, 0.f, 1.f),
                                            50.f,
                                            0.4f,
                                            0.4f,
                                            12,
                                            FLinearColor::Yellow);

        auto executorCreature = Cast<ACreatureBase>(GetExecutorActor());
        FLinearColor partyMemberColor = executorCreature != nullptr ? executorCreature->GetPartyMemberColor() : FLinearColor::Black;

        FIntPoint finalTile = mSetupData.mMovementTiles.Num() > 0 ? mSetupData.mMovementTiles.Last() : GetExecutorPlacement();
        TArray<FIntPoint> patternTiles = mTilePattern->GetPatternTiles(finalTile,
                                                                       mSetupData.mExecutionDirection);
        for (const FIntPoint& tile : patternTiles)
        {
            FVector tilePosition = passMap->GetTilePositionWorld(tile);
            FVector boxExtent(50.f);
            UKismetSystemLibrary::DrawDebugBox(this, tilePosition, boxExtent, partyMemberColor);
        }

        for (int32 i = 0; i < mVictims.Num(); ++i)
        {
            const FVictimInfo& victim = mVictims[i];
            if (victim.mArenaUnit.IsValid())
            {
                FLinearColor victimColor = FLinearColor::LerpUsingHSV(FLinearColor::Red,
                                                                      FLinearColor::Green,
                                                                      (float)i / (float)(mVictims.Num() - 1));

                FVector victimLocation = victim.mArenaUnit->GetActorLocation();
                UKismetSystemLibrary::DrawDebugCapsule(this,
                                                       victimLocation + FVector(0.f, 0.f, 100.f),
                                                       100.f,
                                                       35.f,
                                                       FRotator::ZeroRotator,
                                                       victimColor);

                FString victimStatStr = FString::FromInt(i);
                UKismetSystemLibrary::DrawDebugString(this,
                                                      victimLocation + FVector(0.f, 0.f, 200.f),
                                                      victimStatStr,
                                                      nullptr,
                                                      victimColor);
            }
        }
    }
    else if (mIsSetupVisualizationEnabled &&
             mTilePattern != nullptr)
    {
        auto executorCreature = Cast<ACreatureBase>(GetExecutorActor());
        FLinearColor partyMemberColor = executorCreature != nullptr ? executorCreature->GetPartyMemberColor() : FLinearColor::Black;

        APassMap* passMap = GetExecutorTiledMovement()->GetPassMap();
        for (int32 i = 0; i < mSetupData.mMovementTiles.Num(); ++i)
        {
            const FIntPoint& tile = mSetupData.mMovementTiles[i];
            FVector tilePosition = passMap->GetTilePositionWorld(tile);
            float lerpFactor = (float)i / (float)mSetupData.mMovementTiles.Num();
            FLinearColor linearColor = FLinearColor::LerpUsingHSV(FLinearColor::Green, FLinearColor::Yellow, lerpFactor);
            UKismetSystemLibrary::DrawDebugSphere(this, tilePosition, 25.f, 12, linearColor);
        }

        FIntPoint finalTile = mSetupData.mMovementTiles.Num() > 0 ? mSetupData.mMovementTiles.Last() : GetExecutorPlacement();
        FVector finalTileLocation = passMap->GetTilePositionWorld(finalTile);

        FRotator directionRot = passMap->GetDirectionRotation(mSetupData.mExecutionDirection);
        FVector lineStart = finalTileLocation + FVector(0.f, 0.f, 50.f);
        FVector lineEnd = finalTileLocation + directionRot.Vector() * 200.0f + FVector(0.f, 0.f, 50.f);
        UKismetSystemLibrary::DrawDebugLine(this, lineStart, lineEnd, FLinearColor::Green, 0.f, 5.f);

        TArray<FIntPoint> patternTiles = mTilePattern->GetPatternTiles(finalTile,
                                                                       mSetupData.mExecutionDirection);
        for (const FIntPoint& tile : patternTiles)
        {
            FVector tilePosition = passMap->GetTilePositionWorld(tile);
            FVector boxExtent(50.f);
            UKismetSystemLibrary::DrawDebugBox(this, tilePosition, boxExtent, partyMemberColor);
        }
    }
}

void AAction_SeparateMovement_Attack::OnStartExecution_Implementation()
{
    UTiledMovement* tiledMovement = GetExecutorTiledMovement();
    APassMap* passMap = tiledMovement->GetPassMap();
    
    FRotator directionRot = passMap->GetDirectionRotation(mSetupData.mExecutionDirection);
    tiledMovement->RotateTo(directionRot.Yaw);
    
    if (mWeaponClass != nullptr)
    {
        UWeaponOperator* weaponOperator = IArenaUnit_Interface::Execute_GetWeaponOperator(GetExecutorActor());
        if (weaponOperator != nullptr)
        {
            weaponOperator->SpawnAndEquipWeapon(mWeaponClass);
        }
    }

    if (mAttackAnimation != NAME_None)
    {
        UAnimationManager* animationManager = IArenaUnit_Interface::Execute_GetAnimationManager(GetExecutorActor());
        if (animationManager != nullptr)
        {
            UAnimationManager::FDelegateSingleActionAnimationFinished delegate;
            delegate.BindDynamic(this, &AAction_SeparateMovement_Attack::OnAttackAnimationFinished);
            animationManager->TryPlaySingleActionAnimationWithCallback(mAttackAnimation,
                                                                       delegate);
        }
    }
}

void AAction_SeparateMovement_Attack::OnStopExecution_Implementation()
{
    //  ???
}

void AAction_SeparateMovement_Attack::ApplyInner()
{
    if (mTilePattern == nullptr)
    {
        return;
    }

    APassMap* passMap = GetExecutorTiledMovement()->GetPassMap();
    FIntPoint finalTile = GetFinalTile();
    int32 exectuionDirection = GetActionSetupData().mExecutionDirection;

    TArray<FIntPoint> patternTiles = mTilePattern->GetPatternTiles(finalTile, exectuionDirection);

    mVictims.Empty();
    for (const FIntPoint& tile : patternTiles)
    {
        AActor* occupant = passMap->GetOccupant_Safe(tile, EOccupationSlot::OccupationSlot_ArenaUnit);
        if (occupant != nullptr &&
            occupant->Implements<UArenaUnit_Interface>())
        {
            FVictimInfo potentialVictim = { occupant, tile };
            if (ProcessPotentialVictim(potentialVictim))
            {
                mVictims.Push(potentialVictim);
            }
        }
    }

    PrioritizeVictims();

    if (mVictims.Num() > 0)
    {
        AffectVictims();
    }
}

bool AAction_SeparateMovement_Attack::ProcessPotentialVictim(const FVictimInfo& victimInfo)
{
    if (mIgnoreHighAndLowObstacle)
    {
        return true;
    }

    APassMap* passMap = GetExecutorTiledMovement()->GetPassMap();

    FIntPoint highObstacleTile;
    bool isHighObstacleDetected = passMap->DirectVisibility(GetFinalTile(),
                                                            victimInfo.mTile,
                                                            MakeBitMask(EOccupationSlot::OccupationSlot_StaticHighObstacle),
                                                            highObstacleTile) == false;

    if (isHighObstacleDetected)
    {
        return false;
    }
    else if (mIgnoreLowObstacle)
    {
        return true;
    }

    FIntPoint lowObstacleTile;
    bool isLowObstacleDetected = passMap->DirectVisibility(GetFinalTile(),
                                                           victimInfo.mTile,
                                                           MakeBitMask(EOccupationSlot::OccupationSlot_StaticLowObstacle),
                                                           lowObstacleTile) == false;

    return isLowObstacleDetected == false;
}

void AAction_SeparateMovement_Attack::PrioritizeVictims()
{
    FIntPoint executorFinalTile = GetFinalTile();
    int32 myExecutionPriority = GetExecutionPriority();
    APassMap* passMap = GetExecutorTiledMovement()->GetPassMap();
    auto CompareTargetsByDistanceAndExecutionPriority = [executorFinalTile, myExecutionPriority, passMap](const FVictimInfo& left, const FVictimInfo& right) {
        int32 leftTileDistance = passMap->GetDistance(executorFinalTile, left.mTile);
        int32 rightTileDistance = passMap->GetDistance(executorFinalTile, right.mTile);

        if (leftTileDistance > rightTileDistance)
        {
            return false;
        }
        else if (leftTileDistance < rightTileDistance)
        {
            return true;
        }

        UActionManager* leftActionManager = IArenaUnit_Interface::Execute_GetActionManager(left.mArenaUnit.Get());
        int32 leftExecutionPriority = leftActionManager != nullptr ? leftActionManager->GetActionToExecute()->GetExecutionPriority() : 0;

        UActionManager* rightActionManager = IArenaUnit_Interface::Execute_GetActionManager(right.mArenaUnit.Get());
        int32 rightExecutionPriority = rightActionManager != nullptr ? rightActionManager->GetActionToExecute()->GetExecutionPriority() : 0;

        return (leftExecutionPriority < myExecutionPriority && rightExecutionPriority < myExecutionPriority) ?
            leftExecutionPriority > rightExecutionPriority : leftExecutionPriority < rightExecutionPriority;
    };
    mVictims.Sort(CompareTargetsByDistanceAndExecutionPriority);
}

void AAction_SeparateMovement_Attack::OnAttackAnimationFinished()
{
    StopExecution();
}

void AAction_SeparateMovement_Attack::AffectVictims()
{
    for (const FVictimInfo& victim : mVictims)
    {
        if (victim.mArenaUnit.IsValid())
        {
            UDamageable* damageable = IArenaUnit_Interface::Execute_GetDamageable(victim.mArenaUnit.Get());
            if (damageable != nullptr)
            {
                UArenaUnitAttributes* executorAttributes = IArenaUnit_Interface::Execute_GetArenaUnitAttributes(victim.mArenaUnit.Get());
                FDamageParameters damage = mDamage;
                if (executorAttributes != nullptr)
                {
                    damage.mPhysical += executorAttributes->GetStrength();
                    damage.mMagical += executorAttributes->GetIntelligence();
                }

                FDamageParameters outDealedDamage;
                damageable->Damage(damage, GetExecutorActor(), outDealedDamage);

                UDamageable* visualDamageable = IArenaUnit_Interface::Execute_GetVisualDamageable(victim.mArenaUnit.Get());
                if (visualDamageable != nullptr)
                {
                    FDamageParameters outDamage;
                    visualDamageable->Damage(damage, GetExecutorActor(), outDamage);
                }

                ACreatureBase* executorCreature = Cast<ACreatureBase>(GetExecutorActor());
                ACreatureBase* victimCreature = Cast<ACreatureBase>(victim.mArenaUnit.Get());
                if (executorCreature != nullptr &&
                    victimCreature != nullptr)
                {
                    UBattleLog* battleLog = UArenaUtilities::GetBattleLog(this);
                    auto battleLogItem = battleLog->CreateBattleLogItem_DamageCreature(executorCreature,
                                                                                       victimCreature,
                                                                                       outDealedDamage);
                    battleLog->PushBattleLogItem(battleLogItem);
                }

                UE_LOG(ArenaCommonLog, Log, TEXT("%s with action %s damages %s (-%i HP)."),
                       *GetExecutorActor()->GetName(),
                       *GetName(),
                       *victim.mArenaUnit->GetName(),
                       outDealedDamage.ComponentsSum());
            }

            UStatusEffectsManager* statusEffectsManager = IArenaUnit_Interface::Execute_GetStatusEffectsManager(victim.mArenaUnit.Get());
            if (statusEffectsManager != nullptr)
            {
                for (const FStatusEffectTask& statusEffectTask : mStatusEffectTasks)
                {
                    statusEffectsManager->ExecuteStatusEffectTask(statusEffectTask);
                }
            }
        }
    }
}
