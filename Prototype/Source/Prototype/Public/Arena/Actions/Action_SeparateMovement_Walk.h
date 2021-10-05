#pragma once


#include "CoreMinimal.h"
#include "Arena/Action_SeparateMovement_Base.h"
#include "Action_SeparateMovement_Walk.generated.h"


UCLASS()
class AAction_SeparateMovement_Walk
    : public AAction_SeparateMovement_Base
{
    GENERATED_BODY()

public:

    AAction_SeparateMovement_Walk(const FObjectInitializer& initializer = FObjectInitializer::Get());

    void PrepareForPlanning_Implementation() override;

private:

    void Tick(float deltaSeconds) override;

    void OnStartExecution_Implementation() override;

    void OnStopExecution_Implementation() override;

protected:

    void ApplyInner() override;

};