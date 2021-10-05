#pragma once


#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ArenaUnitAttributes.generated.h"


UCLASS()
class UArenaUnitAttributes
    : public UActorComponent
{
    GENERATED_BODY()

public:

    UFUNCTION(BlueprintPure, Category = "Arena Unit")
    int32 GetAgility() const;

    UFUNCTION(BlueprintPure, Category = "Arena Unit")
    int32 GetAgilityBase() const;

    UFUNCTION(BlueprintCallable, Category = "Arena Unit")
    void SetAgilityAdditional(int32 agility);

    UFUNCTION(BlueprintPure, Category = "Arena Unit")
    int32 GetAgilityAdditional() const;

    UFUNCTION(BlueprintPure, Category = "Arena Unit")
    int32 GetStrength() const;

    UFUNCTION(BlueprintPure, Category = "Arena Unit")
    int32 GetStrengthBase() const;

    UFUNCTION(BlueprintCallable, Category = "Arena Unit")
    void SetStrengthAdditional(int32 strength);

    UFUNCTION(BlueprintPure, Category = "Arena Unit")
    int32 GetStrengthAdditional() const;

    UFUNCTION(BlueprintPure, Category = "Arena Unit")
    int32 GetIntelligence() const;

    UFUNCTION(BlueprintPure, Category = "Arena Unit")
    int32 GetIntelligenceBase() const;

    UFUNCTION(BlueprintCallable, Category = "Arena Unit")
    void SetIntelligenceAdditional(int32 intelligence);

    UFUNCTION(BlueprintPure, Category = "Arena Unit")
    int32 GetIntelligenceAdditional() const;

    UFUNCTION(BlueprintPure, Category = "Arena Unit")
    int32 GetSpeed() const;

    UFUNCTION(BlueprintPure, Category = "Arena Unit")
    int32 GetSpeedBase() const;

    UFUNCTION(BlueprintCallable, Category = "Arena Unit")
    void SetSpeedAdditional(int32 speed);

    UFUNCTION(BlueprintPure, Category = "Arena Unit")
    int32 GetSpeedAdditional() const;

protected:

    UPROPERTY(EditAnywhere, Category = "Arena Unit", Meta = (DisplayName = "Agility"))
    int32 mAgilityBase = 0;

    int32 mAgilityAdditional = 0;

    UPROPERTY(EditAnywhere, Category = "Arena Unit", Meta = (DisplayName = "Strength"))
    int32 mStrengthBase = 0;

    int32 mStrengthAdditional = 0;

    UPROPERTY(EditAnywhere, Category = "Arena Unit", Meta = (DisplayName = "Intelligence"))
    int32 mIntelligenceBase = 0;

    int32 mIntelligenceAdditional = 0;

    UPROPERTY(EditAnywhere, Category = "Arena Unit", Meta = (DisplayName = "Speed"))
    int32 mSpeedBase = 0;

    int32 mSpeedAdditional = 0;
};