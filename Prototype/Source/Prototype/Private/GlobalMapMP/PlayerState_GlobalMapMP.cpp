

#include "GlobalMapMP/PlayerState_GlobalMapMP.h"
#include "Net/UnrealNetwork.h"


void APlayerState_GlobalMapMP::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(APlayerState_GlobalMapMP, mScore);
}
