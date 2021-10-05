
#include "ActionScript_ManageParty.h"
#include "UnknownCommon.h"
#include "PlayerController_GlobalMap.h"
#include "GMPawn.h"
#include "ProgressManager.h"


AActionScript_ManageParty::AActionScript_ManageParty()
{
    mActionScriptID = "ManageParty";
}

void AActionScript_ManageParty::ExecuteInner_Implementation(const FName& scenarioID)
{
    const auto* pManagePartyScenario = mpScenariosTable->FindRow<FManagePartyScenario>(scenarioID, scenarioID.ToString() + " manage party scenario is not found.");
    check(pManagePartyScenario != nullptr && "Can't execute ManagerParty action script. Scenario is invalid.");
    AGMPawn* pGMPawn = UUnknownCommon::GetGlobalMapPlayerController(this)->GetGMPawn();
    int32 statusProgVar = -1;
    if (pManagePartyScenario->PartyMemberOperation == EPartyMemberOperation::PartyMemberOperation_Add)
    {
        pGMPawn->AddPartyMember(pManagePartyScenario->CreatureName);
        statusProgVar = 1;
        //pManagePartyScenario->StatusProgressVariable
    }
    else if (pManagePartyScenario->PartyMemberOperation == EPartyMemberOperation::PartyMemberOperation_Remove)
    {
        pGMPawn->RemovePartyMember(pManagePartyScenario->CreatureName);
        statusProgVar = 0;
    }
    if (pManagePartyScenario->StatusProgressVariable.IsNone() == false)
    {
        FProgressVariableChange progVarChange;
        progVarChange.Operation = EProgressVariableOperation::ProgressVariableOperation_Set;
        progVarChange.ProgressVariable = pManagePartyScenario->StatusProgressVariable;
        progVarChange.Value = statusProgVar;
        bool doesExist(false);
        UUnknownCommon::GetProgressManager(this)->ChangeProgressVariable(progVarChange, doesExist);
    }
    FinishExecution(true);
}
