

#include "Arena/Actions/Action_SeparateMovement_Walk.h"
#include "Arena/ArenaUnit_Interface.h"
#include "Arena/ArenaUnitAttributes.h"
#include "Arena/PassMap.h"
#include "Arena/TiledMovement.h"
#include "Kismet/KismetSystemLibrary.h"
#include "TimerManager.h"


AAction_SeparateMovement_Walk::AAction_SeparateMovement_Walk(const FObjectInitializer& initializer)
    : Super(initializer)
{
    PrimaryActorTick.bCanEverTick = true;
}

void AAction_SeparateMovement_Walk::PrepareForPlanning_Implementation()
{
    Super::PrepareForPlanning_Implementation();

    UArenaUnitAttributes* arenaUnitAttributes = IArenaUnit_Interface::Execute_GetArenaUnitAttributes(GetExecutorActor());
    if (arenaUnitAttributes != nullptr)
    {
        SetMovementRangeAdditional(arenaUnitAttributes->GetSpeed());
    }
}

void AAction_SeparateMovement_Walk::Tick(float deltaSeconds)
{
    Super::Tick(deltaSeconds);

    if (IsSetup() == false)
    {
        return;
    }

    if (mIsExecuting)
    {
        FVector executorLocation = GetExecutorActor()->GetActorLocation();
        UKismetSystemLibrary::DrawDebugCone(this,
                                            executorLocation + FVector(0.f, 0.f, 200.f),
                                            FVector(0.f, 0.f, 1.f),
                                            50.f,
                                            0.4f,
                                            0.4f,
                                            12,
                                            FLinearColor::Yellow);
    }
    else if (mIsSetupVisualizationEnabled)
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
    }
}

void AAction_SeparateMovement_Walk::OnStartExecution_Implementation()
{
    FTimerManager& timerManager = GetWorld()->GetTimerManager();
    timerManager.SetTimerForNextTick(this, &AAction_SeparateMovement_Walk::StopExecution);
}

void AAction_SeparateMovement_Walk::OnStopExecution_Implementation()
{

}

void AAction_SeparateMovement_Walk::ApplyInner()
{
    //  Just walk. Nothing to apply.
}
