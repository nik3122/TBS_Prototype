
#include "Arena/Damage.h"


FDamageParameters::FDamageParameters()
    : mAbsolute(0)
    , mPhysical(0)
    , mMagical(0)
    , mPoison(0)
    , mFire(0)
{
}

FDamageParameters::FDamageParameters(int32 absolute, 
                                     int32 physical, 
                                     int32 magical, 
                                     int32 poison, 
                                     int32 fire)
    : mAbsolute(absolute)
    , mPhysical(physical)
    , mMagical(magical)
    , mPoison(poison)
    , mFire(fire)
{
}

int32 FDamageParameters::ComponentsSum() const
{
    return mAbsolute + mPhysical + mMagical + mPoison + mFire;
}

FDamageParameters FDamageParameters::operator+(const FDamageParameters& right) const
{
    const FDamageParameters& left = *this;
    FDamageParameters result;
    result.mAbsolute = left.mAbsolute + right.mAbsolute;
    result.mPhysical = left.mPhysical + right.mPhysical;
    result.mMagical = left.mMagical + right.mMagical;
    result.mPoison = left.mPoison + right.mPoison;
    result.mFire = left.mFire + right.mFire;
    return result;
}

FDamageParameters FDamageParameters::operator-(const FDamageParameters& right) const
{
    const FDamageParameters& left = *this;
    FDamageParameters result;
    result.mAbsolute = left.mAbsolute - right.mAbsolute;
    result.mPhysical = left.mPhysical - right.mPhysical;
    result.mMagical = left.mMagical - right.mMagical;
    result.mPoison = left.mPoison - right.mPoison;
    result.mFire = left.mFire - right.mFire;
    return result;
}

FDamageParameters FDamageParameters::operator*(const int32& integer) const
{
    const FDamageParameters& left = *this;
    FDamageParameters result;
    result.mAbsolute = left.mAbsolute * integer;
    result.mPhysical = left.mPhysical * integer;
    result.mMagical = left.mMagical * integer;
    result.mPoison = left.mPoison * integer;
    result.mFire = left.mFire * integer;
    return result;
}

FDamageParameters FDamageParameters::operator/(const int32& integer) const
{
    const FDamageParameters& left = *this;
    FDamageParameters result;
    result.mAbsolute = left.mAbsolute / integer;
    result.mPhysical = left.mPhysical / integer;
    result.mMagical = left.mMagical / integer;
    result.mPoison = left.mPoison / integer;
    result.mFire = left.mFire / integer;
    return result;
}

FDamageParameters& FDamageParameters::operator+=(const FDamageParameters& right)
{
    mAbsolute += right.mAbsolute;
    mPhysical += right.mPhysical;
    mMagical += right.mMagical;
    mPoison += right.mPoison;
    mFire += right.mFire;
    return *this;
}

FDamageParameters& FDamageParameters::operator-=(const FDamageParameters& right)
{
    mAbsolute -= right.mAbsolute;
    mPhysical -= right.mPhysical;
    mMagical -= right.mMagical;
    mPoison -= right.mPoison;
    mFire -= right.mFire;
    return *this;
}

FDamageParameters& FDamageParameters::operator*=(const int32& integer)
{
    mAbsolute *= integer;
    mPhysical *= integer;
    mMagical *= integer;
    mPoison *= integer;
    mFire *= integer;
    return *this;
}

FDamageParameters& FDamageParameters::operator/=(const int32& integer)
{
    mAbsolute /= integer;
    mPhysical /= integer;
    mMagical /= integer;
    mPoison /= integer;
    mFire /= integer;
    return *this;
}

FDamageParameters UDamageOperator::NullDamage()
{
    return {};
}

int32 UDamageOperator::DamageComponentsSum(const FDamageParameters& damage)
{
    return damage.ComponentsSum();
}

int32 UDamageOperator::Get100PercentMultiplier()
{
    return 10;
}

int32 UDamageOperator::Get100PercentValue()
{
    return 100 * Get100PercentMultiplier();
}

int32 UDamageOperator::ClampValue(int32 value)
{
    return FMath::Max(value, 0);
}

int32 UDamageOperator::MakePercentageValue(float percents)
{
    return FMath::CeilToInt(percents * static_cast<float>(Get100PercentMultiplier()));
}

int32 UDamageOperator::ModifyValuePercentage(int32 value, int32 scaledPercentage)
{
    return (scaledPercentage * value) / Get100PercentValue();
}

FDamageParameters UDamageOperator::CalculateDamage(const FDamageParameters& damage,
                                                   const FDamageParameters& resist,
                                                   const FDamageParameters& defence)
{
    int32 physical = ModifyValuePercentage(FMath::Max(FMath::Abs(damage.mPhysical) - FMath::Abs(defence.mPhysical), 0), FMath::Max(Get100PercentValue() - resist.mPhysical, 0));
    int32 magical = ModifyValuePercentage(FMath::Max(FMath::Abs(damage.mMagical) - FMath::Abs(defence.mMagical), 0), FMath::Max(Get100PercentValue() - resist.mMagical, 0));
    int32 poison = ModifyValuePercentage(FMath::Max(FMath::Abs(damage.mPoison) - FMath::Abs(defence.mPoison), 0), FMath::Max(Get100PercentValue() - resist.mPoison, 0));
    int32 fire = ModifyValuePercentage(FMath::Max(FMath::Abs(damage.mFire) - FMath::Abs(defence.mFire), 0), FMath::Max(Get100PercentValue() - resist.mFire, 0));
    return FDamageParameters(damage.mAbsolute, physical, magical, poison, fire);
}

FDamageParameters UDamageOperator::Get100PercentDamage()
{
    return FDamageParameters{
        Get100PercentValue(),
        Get100PercentValue(),
        Get100PercentValue(),
        Get100PercentValue(),
        Get100PercentValue(),
    };
}

FDamageParameters UDamageOperator::MakePercentageDamage(float percent)
{
    FDamageParameters result;
    result.mAbsolute = result.mPhysical = result.mMagical = result.mPoison = result.mFire = MakePercentageValue(percent);
    return result;
}

FDamageParameters UDamageOperator::ModifyDamage(const FDamageParameters& baseDamage,
                                                const FAttackModifier& attackModifier)
{
    FDamageParameters result;
    const FDamageParameters& additional = attackModifier.mDamageAdditionalBonus;
    const FDamageParameters& percentage = attackModifier.mDamagePercentageBonus;
    result.mAbsolute = ModifyValuePercentage(baseDamage.mAbsolute + additional.mAbsolute, FMath::Max(percentage.mAbsolute, 0));
    result.mPhysical = ModifyValuePercentage(baseDamage.mPhysical + additional.mPhysical, FMath::Max(percentage.mPhysical, 0));
    result.mMagical = ModifyValuePercentage(baseDamage.mMagical + additional.mMagical, FMath::Max(percentage.mMagical, 0));
    result.mPoison = ModifyValuePercentage(baseDamage.mPoison + additional.mPoison, FMath::Max(percentage.mPoison, 0));
    result.mFire = ModifyValuePercentage(baseDamage.mFire + additional.mFire, FMath::Max(percentage.mFire, 0));
    return result;
}

FDamageParameters UDamageOperator::ModifyDamageAddition(const FDamageParameters& baseDamage,
                                                        const FDamageParameters& additional)
{
    FDamageParameters result;
    result.mAbsolute = baseDamage.mAbsolute + additional.mAbsolute;
    result.mPhysical = baseDamage.mPhysical + additional.mPhysical;
    result.mMagical = baseDamage.mMagical + additional.mMagical;
    result.mPoison = baseDamage.mPoison + additional.mPoison;
    result.mFire = baseDamage.mFire + additional.mFire;
    return result;
}

FAttackModifier UDamageOperator::SumAttackModifiers(const FAttackModifier& left, const FAttackModifier& right)
{
    FAttackModifier result;
    result.mDamageAdditionalBonus = ModifyDamageAddition(left.mDamageAdditionalBonus, right.mDamageAdditionalBonus);
    result.mDamagePercentageBonus = ModifyDamageAddition(left.mDamagePercentageBonus, right.mDamagePercentageBonus);
    return result;
}

FDamageParameters UDamageOperator::DamageAddDamage(const FDamageParameters& leftDamage,
                                                   const FDamageParameters& rightDamage)
{
    return leftDamage + rightDamage;
}

FDamageParameters UDamageOperator::DamageSubtractDamage(const FDamageParameters& leftDamage,
                                                        const FDamageParameters& rightDamage)
{
    return leftDamage - rightDamage;
}

FDamageParameters UDamageOperator::DamageMultiplyInteger(const FDamageParameters& damage,
                                                         int32 integer)
{
    return damage * integer;
}

FDamageParameters UDamageOperator::DamageDivideInteger(const FDamageParameters& damage,
                                                       int32 integer)
{
    return damage / integer;
}

FDamageParameters UDamageOperator::NegateDamage(const FDamageParameters& damage)
{
    return NullDamage() - damage;
}
