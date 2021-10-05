
#include "ActionScript_CampTravel.h"
#include "PlayerController_GlobalMap.h"
#include "Kismet/GameplayStatics.h"


AActionScript_CampTravel::AActionScript_CampTravel()
    : mpPlayerController(nullptr)
{
    mActionScriptID = "CampTravel";
}

void AActionScript_CampTravel::ExecuteInner_Implementation(const FName& scenarioID)
{
    FCampTravelScenario* pCampTravelScenario= mpScenariosTable->FindRow<FCampTravelScenario>(scenarioID, scenarioID.ToString() + " camp travel scenario is not found.");
    check(pCampTravelScenario != nullptr && "Camp travel scenario isn't found.");

    mpPlayerController = Cast<APlayerController_GlobalMap>(UGameplayStatics::GetPlayerController(this, 0));
    check(mpPlayerController != nullptr && "Player controller isn't PlayerController_GlobalMap.");
    mpPlayerController->mEventTravelFinished.AddDynamic(this, &AActionScript_CampTravel::OnTravelFinished);
    //pPlayerController->mEventTravelFinished.AddRaw //(this, &AActionScript_CampTravel::)
    mpPlayerController->StartCampTravelByID(
        pCampTravelScenario->TargetCampID,
        pCampTravelScenario->CampTravelType,
        pCampTravelScenario->FirstActionBlockID
    );
    
}

void AActionScript_CampTravel::OnTravelFinished()
{
    mpPlayerController->mEventTravelFinished.RemoveDynamic(this, &AActionScript_CampTravel::OnTravelFinished);
    FinishExecution(true);
}
