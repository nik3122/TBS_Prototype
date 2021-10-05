

#include "ActionScript_ManipulateRoad.h"
#include "GameMode_GlobalMap.h"
#include "GMRoad.h"
#include "UnknownCommon.h"


AActionScript_ManipulateRoad::AActionScript_ManipulateRoad()
{
    mActionScriptID = "ManipulateRoad";
}

void AActionScript_ManipulateRoad::ExecuteInner_Implementation(const FName& scenarioID)
{
    const auto* pManipulateRoadScenario = mpScenariosTable->FindRow<FManipulateRoadScenario>(scenarioID, scenarioID.ToString() + " manipulate road scenario is not found.");
    check(pManipulateRoadScenario != nullptr && "Can't execute ManagerParty action script. Scenario is invalid.");
    AGMRoad* pRoad = UUnknownCommon::GetGlobalMapGameMode(this)->TryGetRoad(pManipulateRoadScenario->RoadName);
    if (pRoad == nullptr)
    {
        UE_LOG(GlobalMapCommonLog, Error, TEXT("AS ManipulateRoad FAILED as %s road wasn't found."), *pManipulateRoadScenario->RoadName.ToString());
    }
    else
    {
        pRoad->SetAvailable(pManipulateRoadScenario->Operation == ERoadManipulation::RoadManipulation_Open);
    }
    //  ~TODO~?~ Wait until potential level sequence will be finished.
    FinishExecution(true);
}
