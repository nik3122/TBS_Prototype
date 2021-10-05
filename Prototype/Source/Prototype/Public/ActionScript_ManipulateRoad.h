#pragma once


#include "CoreMinimal.h"
#include "ActionScript_Base.h"
#include "Engine/DataTable.h"
#include "ActionScript_ManipulateRoad.generated.h"


UENUM(BlueprintType)
enum class ERoadManipulation : uint8
{
    RoadManipulation_Open,
    RoadManipulation_Close,
};


USTRUCT()
struct FManipulateRoadScenario
    : public FTableRowBase
{
    GENERATED_BODY()

public:

    UPROPERTY(EditAnywhere)
    FName RoadName;

    UPROPERTY(EditAnywhere)
    ERoadManipulation Operation;
};


UCLASS()
class AActionScript_ManipulateRoad
    : public AActionScript_Base
{
    GENERATED_BODY()

public:

    AActionScript_ManipulateRoad();

protected:

    void ExecuteInner_Implementation(const FName& scenarioID) override;
};