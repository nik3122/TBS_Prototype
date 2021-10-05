

#include "Arena/Action_SeparateMovement_Base.h"
#include "Arena/ArenaUtilities.h"
#include "Arena/CreatureBase.h"
#include "Arena/CreatureGhost.h"
#include "Arena/PassMap.h"
#include "Arena/TiledMovement.h"
#include "Kismet/KismetSystemLibrary.h"
#include "TileMaps/HexTilePattern.h"


AAction_SeparateMovement_Base::AAction_SeparateMovement_Base(const FObjectInitializer& initializer)
    : Super(initializer)
{
}

bool AAction_SeparateMovement_Base::Setup()
{
    Reset();

    mIsSetup = true;

    if (mIsSetupVisualizationEnabled)
    {
        UpdateSetupVisualization();
    }

    return true;
}


void AAction_SeparateMovement_Base::Apply()
{
    ApplyInner();

    K2_Apply();
}

bool AAction_SeparateMovement_Base::CanBeSetupAt_Implementation(const FIntPoint& potentialSetupTile) const
{
    return true;
}

UHexTilePattern* AAction_SeparateMovement_Base::GetTilePattern() const
{
    return mTilePattern;
}

void AAction_SeparateMovement_Base::ApplyInner()
{
    UE_LOG(ArenaCommonLog, Warning, TEXT("%s::ApplyInner() should be implemented."),
           *GetClass()->GetName());
}

void AAction_SeparateMovement_Base::EnableSetupVisualization_Inner()
{
    auto executorCreature = Cast<ACreatureBase>(GetExecutorActor());
    if (executorCreature == nullptr)
    {
        return;
    }

    ACreatureGhost* creatureGhost = executorCreature->GetCreatureGhost();
    if (creatureGhost == nullptr)
    {
        return;
    }

    if (IsSetup() == false ||
        mSetupData.mMovementTiles.Num() == 0)
    {
        creatureGhost->SetActorHiddenInGame(true);
    }
    else
    {
        const FIntPoint& finalTile = mSetupData.mMovementTiles.Last();
        APassMap* passMap = GetExecutorTiledMovement()->GetPassMap();
        FVector finalLoc = passMap->GetTilePositionWorld(finalTile);
        FRotator finalRot = passMap->GetDirectionRotation(mSetupData.mExecutionDirection);
        creatureGhost->SetActorLocationAndRotation(finalLoc,
                                                   finalRot);
        creatureGhost->SetActorHiddenInGame(false);
    }
}

void AAction_SeparateMovement_Base::DisableSetupVisualization_Inner()
{
    auto executorCreature = Cast<ACreatureBase>(GetExecutorActor());
    if (executorCreature == nullptr)
    {
        return;
    }

    ACreatureGhost* creatureGhost = executorCreature->GetCreatureGhost();
    if (creatureGhost == nullptr)
    {
        return;
    }

    creatureGhost->SetActorHiddenInGame(true);
}

void AAction_SeparateMovement_Base::UpdateSetupVisualization()
{
    if (mSetupData.mMovementTiles.Num() == 0)
    {
        return;
    }

    auto executorCreature = Cast<ACreatureBase>(GetExecutorActor());
    if (executorCreature == nullptr)
    {
        return;
    }

    ACreatureGhost* creatureGhost = executorCreature->GetCreatureGhost();
    if (creatureGhost == nullptr)
    {
        return;
    }

    if (IsSetup() == false ||
        mSetupData.mMovementTiles.Num() == 0)
    {
        creatureGhost->SetActorHiddenInGame(true);
    }
    else
    {
        const FIntPoint& finalTile = mSetupData.mMovementTiles.Last();
        APassMap* passMap = GetExecutorTiledMovement()->GetPassMap();
        FVector finalLoc = passMap->GetTilePositionWorld(finalTile);
        FRotator finalRot = passMap->GetDirectionRotation(mSetupData.mExecutionDirection);
        creatureGhost->SetActorLocationAndRotation(finalLoc,
                                                   finalRot);
        creatureGhost->SetActorHiddenInGame(false);
    }
}

#if WITH_EDITOR

void AAction_SeparateMovement_Base::PostEditChangeProperty(FPropertyChangedEvent& propertyChangedEvent)
{
    FName propertyName = propertyChangedEvent.GetPropertyName();
    if (propertyName == GET_MEMBER_NAME_CHECKED(AAction_SeparateMovement_Base, mActionInitiative))
    {
        mInitiativeBase = mActionInitiative;
    }
    Super::PostEditChangeProperty(propertyChangedEvent);
}

#endif // #if WITH_EDITOR