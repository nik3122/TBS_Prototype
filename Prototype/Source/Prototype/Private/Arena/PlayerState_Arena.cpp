

#include "Arena/PlayerState_Arena.h"
#include "Net/UnrealNetwork.h"


void APlayerState_Arena::BeginPlay()
{
    Super::BeginPlay();

    GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Red, TEXT("APlayerState_Arena::BeginPlay()"));
}

void APlayerState_Arena::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(APlayerState_Arena, mIsPlanningFinished);
    DOREPLIFETIME(APlayerState_Arena, mIsExecutionFinished);
    DOREPLIFETIME(APlayerState_Arena, mEoAWantsToRetry);
}
