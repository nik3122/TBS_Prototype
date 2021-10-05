#pragma once


#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "PlayerState_Arena.generated.h"


UCLASS()
class APlayerState_Arena
    : public APlayerState
{
    GENERATED_BODY()

public:

    UPROPERTY(Replicated,  BlueprintReadOnly, Category = "Arena")
    bool mIsPlanningFinished = false;

    UPROPERTY(Replicated,  BlueprintReadOnly, Category = "Arena")
    bool mIsExecutionFinished = false;

    UPROPERTY(Replicated, BlueprintReadOnly, Category = "Arena")
    bool mEoAWantsToRetry = false;

    FLinearColor mPartyColor = FLinearColor::Black;

public:

    void BeginPlay() override;

    void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

};