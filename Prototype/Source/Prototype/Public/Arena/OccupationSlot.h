#pragma once


#include "OccupationSlot.generated.h"


UENUM(BlueprintType, Meta = (Bitflags))
enum class EOccupationSlot : uint8
{
    OccupationSlot_Static,
    OccupationSlot_StaticLowObstacle,
    OccupationSlot_StaticHighObstacle,
    OccupationSlot_ArenaUnit,
    OccupationSlot_Trap,
    OccupationSlot_Fire,
    OccupationSlot_MovementInterrupter,
    OccupationSlot_MAX,
};

