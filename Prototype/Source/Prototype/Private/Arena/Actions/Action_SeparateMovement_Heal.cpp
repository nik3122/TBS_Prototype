

#include "Arena/Actions/Action_SeparateMovement_Heal.h"
#include "Arena/Actionmanager.h"
#include "Arena/AnimationManager.h"
#include "Arena/ArenaUnitAttributes.h"
#include "Arena/ArenaUtilities.h"
#include "Arena/BattleLog/BattleLog.h"
#include "Arena/BattleLog/BattleLogItem_HealCreature.h"
#include "Arena/CreatureBase.h"
#include "Arena/PassMap.h"
#include "Arena/TiledMovement.h"
#include "Kismet/KismetSystemLibrary.h"
#include "TileMaps/HexTilePattern.h"


AAction_SeparateMovement_Heal::AAction_SeparateMovement_Heal(const FObjectInitializer& initializer)
    : Super(initializer)
{
    PrimaryActorTick.bCanEverTick = true;
}


void AAction_SeparateMovement_Heal::Tick(float deltaSeconds)
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

        for (const FTargetUnitInfo& targetUnit : mTargetUnits)
        {
            if (targetUnit.mArenaUnit.IsValid())
            {
                FVector targetLocation = targetUnit.mArenaUnit->GetActorLocation() + FVector(0.f, 0.f, 100.f);
                UKismetSystemLibrary::DrawDebugCapsule(this,
                                                       targetLocation,
                                                       100.f,
                                                       40.f,
                                                       FRotator::ZeroRotator,
                                                       FLinearColor::Green);
            }
        }
    }
    else if (mIsSetupVisualizationEnabled &&
             mTilePattern != nullptr)
    {
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

        auto executorCreature = Cast<ACreatureBase>(GetExecutorActor());
        FLinearColor partyMemberColor = executorCreature != nullptr ? executorCreature->GetPartyMemberColor() : FLinearColor::Black;

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

void AAction_SeparateMovement_Heal::OnStartExecution_Implementation()
{
    UTiledMovement* tiledMovement = GetExecutorTiledMovement();
    APassMap* passMap = tiledMovement->GetPassMap();

    FRotator directionRot = passMap->GetDirectionRotation(mSetupData.mExecutionDirection);
    tiledMovement->RotateTo(directionRot.Yaw);

    if (mActionAnimation != NAME_None)
    {
        UAnimationManager* animationManager = IArenaUnit_Interface::Execute_GetAnimationManager(GetExecutorActor());
        if (animationManager != nullptr)
        {
            UAnimationManager::FDelegateSingleActionAnimationFinished delegate;
            delegate.BindDynamic(this, &AAction_SeparateMovement_Heal::OnActionAnimationFinished);
            animationManager->TryPlaySingleActionAnimationWithCallback(mActionAnimation,
                                                                       delegate);
        }
    }
}

void AAction_SeparateMovement_Heal::OnStopExecution_Implementation()
{
}

void AAction_SeparateMovement_Heal::ApplyInner()
{
    if (mTilePattern == nullptr)
    {
        return;
    }

    APassMap* passMap = GetExecutorTiledMovement()->GetPassMap();
    FIntPoint finalTile = GetFinalTile();
    int32 exectuionDirection = GetActionSetupData().mExecutionDirection;

    TArray<FIntPoint> patternTiles = mTilePattern->GetPatternTiles(finalTile, exectuionDirection);

    mTargetUnits.Empty();
    for (const FIntPoint& tile : patternTiles)
    {
        AActor* occupant = passMap->GetOccupant_Safe(tile, EOccupationSlot::OccupationSlot_ArenaUnit);
        if (occupant != nullptr &&
            occupant->Implements<UArenaUnit_Interface>())
        {
            FTargetUnitInfo potentialTargetUnit = { occupant, tile };
            if (ProcessPotentialTargetUnit(potentialTargetUnit))
            {
                mTargetUnits.Push(potentialTargetUnit);
            }
        }
    }

    //  Prioritize targets:
    PrioritizeTargetUnits();

    if (mTargetUnits.Num() > 0)
    {
        AffectTargetUnits();
    }
}

bool AAction_SeparateMovement_Heal::ProcessPotentialTargetUnit(const FTargetUnitInfo& targetUnitInfo)
{
    return IArenaUnit_Interface::Execute_GetDamageable(targetUnitInfo.mArenaUnit.Get()) != nullptr;
}

void AAction_SeparateMovement_Heal::PrioritizeTargetUnits()
{
    FIntPoint executorFinalTile = GetFinalTile();
    APassMap* passMap = GetExecutorTiledMovement()->GetPassMap();
    auto CompareTargetsByDistanceAndExecutionPriority = [executorFinalTile, passMap](const FTargetUnitInfo& left, const FTargetUnitInfo& right) {
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
        
        return leftExecutionPriority > rightExecutionPriority;
    };
    mTargetUnits.Sort(CompareTargetsByDistanceAndExecutionPriority);
}

void AAction_SeparateMovement_Heal::AffectTargetUnits()
{
    for (int32 i = 0; i < mTargetUnits.Num() && i < mMaxTargetUnitsNumber; ++i)
    {
        const FTargetUnitInfo& targetUnit = mTargetUnits[i];
        if (targetUnit.mArenaUnit.IsValid())
        {
            UDamageable* damageable = IArenaUnit_Interface::Execute_GetDamageable(targetUnit.mArenaUnit.Get());
            if (damageable != nullptr)
            {
                UArenaUnitAttributes* arenaUnitAttributes = IArenaUnit_Interface::Execute_GetArenaUnitAttributes(targetUnit.mArenaUnit.Get());
                int32 healHP = mHealHP + (arenaUnitAttributes != nullptr ? arenaUnitAttributes->GetIntelligence() : 0);
                int32 oldHP = damageable->GetCurrentHP();
                damageable->Heal(mHealHP, GetExecutorActor());
                int32 newHP = damageable->GetCurrentHP();
                int32 changedHP = newHP - oldHP;

                UDamageable* visualDamageable = IArenaUnit_Interface::Execute_GetVisualDamageable(targetUnit.mArenaUnit.Get());
                if (visualDamageable != nullptr)
                {
                    visualDamageable->Heal(mHealHP, GetExecutorActor());
                }

                auto executorCreature = Cast<ACreatureBase>(GetExecutorActor());
                auto targetCreature = Cast<ACreatureBase>(targetUnit.mArenaUnit.Get());
                if (executorCreature != nullptr &&
                    targetCreature != nullptr)
                {
                    UBattleLog* battleLog = UArenaUtilities::GetBattleLog(this);
                    auto battleLogItem = battleLog->CreateBattleLogItem_HealCreature(executorCreature,
                                                                                     targetCreature,
                                                                                     changedHP);
                    battleLog->PushBattleLogItem(battleLogItem);
                }

                UE_LOG(ArenaCommonLog, Log, TEXT("%s with action %s heals %s (+%i HP)."),
                       *GetExecutorActor()->GetName(),
                       *GetName(),
                       *targetUnit.mArenaUnit->GetName(),
                       changedHP);
            }
        }
    }
}

void AAction_SeparateMovement_Heal::OnActionAnimationFinished()
{
    StopExecution();
}
