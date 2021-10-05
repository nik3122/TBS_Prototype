

#include "Arena/ArenaUnitAttributes.h"


int32 UArenaUnitAttributes::GetAgility() const
{
    return mAgilityBase + mAgilityAdditional;
}

int32 UArenaUnitAttributes::GetAgilityBase() const
{
    return mAgilityBase;
}

void UArenaUnitAttributes::SetAgilityAdditional(int32 agility)
{
    mAgilityAdditional = agility;
}

int32 UArenaUnitAttributes::GetAgilityAdditional() const
{
    return mAgilityAdditional;
}

int32 UArenaUnitAttributes::GetStrength() const
{
    return mStrengthBase + mStrengthAdditional;
}

int32 UArenaUnitAttributes::GetStrengthBase() const
{
    return mStrengthBase;
}

void UArenaUnitAttributes::SetStrengthAdditional(int32 strength)
{
    mStrengthAdditional = strength;
}

int32 UArenaUnitAttributes::GetStrengthAdditional() const
{
    return mStrengthAdditional;
}

int32 UArenaUnitAttributes::GetIntelligence() const
{
    return mIntelligenceBase + mIntelligenceAdditional;
}

int32 UArenaUnitAttributes::GetIntelligenceBase() const
{
    return mIntelligenceBase;
}

void UArenaUnitAttributes::SetIntelligenceAdditional(int32 intelligence)
{
    mIntelligenceAdditional = intelligence;
}

int32 UArenaUnitAttributes::GetIntelligenceAdditional() const
{
    return mIntelligenceAdditional;
}

int32 UArenaUnitAttributes::GetSpeed() const
{
    return mSpeedBase + mSpeedAdditional;
}

int32 UArenaUnitAttributes::GetSpeedBase() const
{
    return mSpeedBase;
}

void UArenaUnitAttributes::SetSpeedAdditional(int32 speed)
{
    mSpeedAdditional = speed;
}

int32 UArenaUnitAttributes::GetSpeedAdditional() const
{
    return mSpeedAdditional;
}
