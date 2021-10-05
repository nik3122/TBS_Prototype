#pragma once


#include "CoreMinimal.h"
#include "Arena/Actions/Action_SeparateMovement_Attack.h"
#include "Arena/Damage.h"
#include "Action_SeparateMovement_Melee.generated.h"


UCLASS()
class AAction_SeparateMovement_Melee
    : public AAction_SeparateMovement_Attack
{
    GENERATED_BODY()

public:

    AAction_SeparateMovement_Melee(const FObjectInitializer& initializer = FObjectInitializer::Get());

};