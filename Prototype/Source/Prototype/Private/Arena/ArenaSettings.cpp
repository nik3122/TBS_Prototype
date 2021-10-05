

#include "Arena/ArenaSettings.h"


UArenaSettings* UArenaSettings::Get()
{
    return CastChecked<UArenaSettings>(UArenaSettings::StaticClass()->GetDefaultObject());
}
