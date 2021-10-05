#pragma once


#include "Weather.generated.h"


/**
    0 - Midnight
    1 - Down
    2 - Sunrise
    3 - Noon
    4 - Afternoon
    5 - Evening
    6 - Night
*/
UENUM(BlueprintType)
enum class EDayTime: uint8
{
    DayTime_Midnight = 0,
    DayTime_Down,
    DayTime_Sunrise,
    DayTime_Noon,
    DayTime_Afternoon,
    DayTime_Evening,
    DayTime_Night,
};


USTRUCT(BlueprintType)
struct FWeatherState
{
    GENERATED_BODY()

public:

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unknown|ArenaFight")
    EDayTime mDayTime = EDayTime::DayTime_Midnight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unknown|ArenaFight")
    bool mIsRaining = false;

    //  100% = 10000 value.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unknown|ArenaFight")
    int32 mPossibilityToMissRangeAttack;
};


UCLASS()
class PROTOTYPE_API AWeather
    : public AActor
{
    GENERATED_BODY()

public:

    void SetupSystem();

    UFUNCTION(BlueprintPure, Category = "Unknown|ArenaFight")
    const FWeatherState& GetWeatherState() const;

    UFUNCTION(BlueprintPure, Category = "Unknown|ArenaFight")
    bool GenerateRangeAttackMiss() const;

protected:

    UPROPERTY(BlueprintReadWrite, Category = "Unknown|ArenaFight")
    FWeatherState mWeatherState;

protected:

    void BeginPlay() override;
};