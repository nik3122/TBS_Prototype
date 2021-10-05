#pragma once


#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "Net/UnrealNetwork.h"
#include "PlayerState_GlobalMapMP.generated.h"


UCLASS()
class APlayerState_GlobalMapMP
    : public APlayerState
{
    GENERATED_BODY()

public:

    UPROPERTY(BlueprintReadWrite, Replicated)
    int32 mScore;

public:

    void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& outLifetimeProps) const override;
};