#pragma once


#include "CoreMinimal.h"
#include "Arena/Actions/Action_SeparateMovement_Attack.h"
#include "Arena/Damage.h"
#include "Action_SeparateMovement_Magic.generated.h"


UCLASS()
class AAction_SeparateMovement_Magic
    : public AAction_SeparateMovement_Attack
{
    GENERATED_BODY()

public:

    AAction_SeparateMovement_Magic(const FObjectInitializer& initializer = FObjectInitializer::Get());

};