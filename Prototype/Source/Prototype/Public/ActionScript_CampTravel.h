#pragma once


#include "CoreMinimal.h"
#include "ActionScript_Base.h"
#include "DialogueSystem.h"
#include "ActionScript_CampTravel.generated.h"


class APlayerController_GlobalMap;


USTRUCT()
struct FCampTravelScenario
    : public FTableRowBase
{
    GENERATED_BODY()

public:

    UPROPERTY(EditAnywhere)
    FName TargetCampID;

    UPROPERTY(EditAnywhere)
    ECampTravel CampTravelType;

    UPROPERTY(EditAnywhere)
    FName FirstActionBlockID = "Default";
};


UCLASS()
class AActionScript_CampTravel
    : public AActionScript_Base
{
    GENERATED_BODY()

public:

    AActionScript_CampTravel();

protected:

    void ExecuteInner_Implementation(const FName& scenarioID) override;

    UFUNCTION()
    void OnTravelFinished();

private:

    APlayerController_GlobalMap* mpPlayerController;
};