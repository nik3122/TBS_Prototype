

#include "Arena/Actions/Action_SeparateMovement_Magic.h"
#include "Arena/ArenaUnit_Interface.h"
#include "Arena/Damageable.h"


AAction_SeparateMovement_Magic::AAction_SeparateMovement_Magic(const FObjectInitializer& initializer)
    : Super(initializer)
{
    mIgnoreHighAndLowObstacle = true;
    mIgnoreLowObstacle = true;
}