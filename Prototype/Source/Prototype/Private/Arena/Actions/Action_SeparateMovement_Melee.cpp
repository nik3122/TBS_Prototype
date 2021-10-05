

#include "Arena/Actions/Action_SeparateMovement_Melee.h"
#include "Arena/ArenaUnit_Interface.h"
#include "Arena/Damageable.h"


AAction_SeparateMovement_Melee::AAction_SeparateMovement_Melee(const FObjectInitializer& initializer)
    : Super(initializer)
{
    mIgnoreHighAndLowObstacle = false;
    mIgnoreLowObstacle = false;
}
