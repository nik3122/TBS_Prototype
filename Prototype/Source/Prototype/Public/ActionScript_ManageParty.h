#pragma once


#include "CoreMinimal.h"
#include "ActionScript_Base.h"
#include "Engine/DataTable.h"
#include "ActionScript_ManageParty.generated.h"


UENUM(BlueprintType)
enum class EPartyMemberOperation : uint8
{
    PartyMemberOperation_Add,
    PartyMemberOperation_Remove,
};


USTRUCT()
struct FManagePartyScenario
    : public FTableRowBase
{
    GENERATED_BODY()

public:

    UPROPERTY(EditAnywhere)
    EPartyMemberOperation PartyMemberOperation;

    UPROPERTY(EditAnywhere)
    FName CreatureName;

    /*
        Progress variable to update with the result of the operation.
        In case the creature becomes the member of the party it is set to 1. Otherwise it is set to 0.
    */
    UPROPERTY(EditAnywhere)
    FName StatusProgressVariable;
};


UCLASS()
class AActionScript_ManageParty
    : public AActionScript_Base
{
    GENERATED_BODY()

public:

    AActionScript_ManageParty();

protected:

    void ExecuteInner_Implementation(const FName& scenarioID) override;
};