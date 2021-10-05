#pragma once

#include "CoreMinimal.h"
#include "Version.h"
#include "Damage.generated.h"


/** This structure contains the main parameters of damage system. */
USTRUCT(BlueprintType)
struct FDamageParameters
{
    GENERATED_BODY()

    DECLARE_CLASS_VERSION(1, 0);

public:

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unknown|Damage")
    int32 mAbsolute = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unknown|Damage")
    int32 mPhysical = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unknown|Damage")
    int32 mMagical = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unknown|Damage")
    int32 mPoison = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unknown|Damage")
    int32 mFire = 0;

public:

    FDamageParameters();

    FDamageParameters(int32 absolute,
                      int32 physical,
                      int32 magical,
                      int32 position,
                      int32 fire);

    int32 ComponentsSum() const;

    FDamageParameters operator+(const FDamageParameters& right) const;

    FDamageParameters operator-(const FDamageParameters& right) const;

    FDamageParameters operator*(const int32& integer) const;

    FDamageParameters operator/(const int32& integer) const;

    FDamageParameters& operator+=(const FDamageParameters& right);

    FDamageParameters& operator-=(const FDamageParameters& right);

    FDamageParameters& operator*=(const int32& integer);

    FDamageParameters& operator/=(const int32& integer);
};


USTRUCT(BlueprintType)
struct FAttackModifier
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unknown|Damage")
    FDamageParameters mDamageAdditionalBonus;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unknown|Damage")
    FDamageParameters mDamagePercentageBonus;

    //UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unknown|Damage")
    //FDamageParameters mDamageMultiplierBonus;
};


/** This is finction library class that manages damage operations. It contains
    the core rules of damage systems like calculating damage counting defence
    and resistance etc.*/
UCLASS()
class PROTOTYPE_API UDamageOperator
    : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:

    UFUNCTION(BlueprintPure, Category = "Unknown|Damage", Meta = (DisplayName = "0 Damage"))
    static FDamageParameters NullDamage();

    UFUNCTION(BlueprintPure, Category = "Unknown|Damage")
    static int32 DamageComponentsSum(const FDamageParameters& damage);

    UFUNCTION(BlueprintPure, Category = "Unknown|Damage")
    static int32 Get100PercentMultiplier();

    UFUNCTION(BlueprintPure, Category = "Unknown|Damage")
    static int32 Get100PercentValue();

    UFUNCTION(BlueprintPure, Category = "Unknown|Damage")
    static int32 ClampValue(int32 value);

    UFUNCTION(BlueprintPure, Category = "Unknown|Damage")
    static int32 MakePercentageValue(float percent);

    UFUNCTION(BlueprintPure, Category = "Unknown|Damage")
    static int32 ModifyValuePercentage(int32 value, int32 scaledPercents);

    UFUNCTION(BlueprintPure, Category = "Unknown|Damage")
    static FDamageParameters CalculateDamage(const FDamageParameters& damage,
                                             const FDamageParameters& resist,
                                             const FDamageParameters& defence);

    UFUNCTION(BlueprintPure, Category = "Unknown|Damage")
    static FDamageParameters Get100PercentDamage();

    UFUNCTION(BlueprintPure, Category = "Unknown|Damage")
    static FDamageParameters MakePercentageDamage(float percent);

    UFUNCTION(BlueprintPure, Category = "Unknown|Damage")
    static FDamageParameters ModifyDamage(const FDamageParameters& baseDamage,
                                          const FAttackModifier& attackModifier);

    UFUNCTION(BlueprintPure, Category = "Unknown|Damage")
    static FDamageParameters ModifyDamageAddition(const FDamageParameters& baseDamage,
                                                  const FDamageParameters& additional);

    UFUNCTION(BlueprintPure, Category = "Unknown|Damage")
    static FAttackModifier SumAttackModifiers(const FAttackModifier& left,
                                              const FAttackModifier& right);

    UFUNCTION(BlueprintPure, Category = "Unknown|Damage", Meta = (DisplayName = "Damage + Damage"))
    static FDamageParameters DamageAddDamage(const FDamageParameters& leftDamage,
                                             const FDamageParameters& rightDamage);

    UFUNCTION(BlueprintPure, Category = "Unknown|Damage", Meta = (DisplayName = "Damage - Damage"))
    static FDamageParameters DamageSubtractDamage(const FDamageParameters& leftDamage,
                                                  const FDamageParameters& rightDamage);

    UFUNCTION(BlueprintPure, Category = "Unknown|Damage", Meta = (DisplayName = "Damage * integer"))
    static FDamageParameters DamageMultiplyInteger(const FDamageParameters& damage,
                                                   int32 integer);

    UFUNCTION(BlueprintPure, Category = "Unknown|Damage", Meta = (DisplayName = "Damage / integer"))
    static FDamageParameters DamageDivideInteger(const FDamageParameters& damage,
                                                 int32 integer);

    UFUNCTION(BlueprintPure, Category = "Unknown|Damage", Meta = (DisplayName = "- Damager"))
    static FDamageParameters NegateDamage(const FDamageParameters& damage);

};