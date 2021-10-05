
#include "ActionScript_LoadArena.h"
#include "UnknownCommon.h"
#include "GameInstance_Unknown.h"
#include "GameMode_GlobalMap.h"
#include "Arena/Party.h"
#include "Arena/CreatureBase.h"
#include "PlayerController_GlobalMap.h"
#include "GMPawn.h"


AActionScript_LoadArena::AActionScript_LoadArena()
{
    mActionScriptID = "LoadArena";
}

void AActionScript_LoadArena::ExecuteInner_Implementation(const FName& scenarioID)
{
    const FLoadArenaScenario* pLoadArenaScenario = mpScenariosTable->FindRow<FLoadArenaScenario>(scenarioID, scenarioID.ToString() + " load arena scenario is not found.");
    check(pLoadArenaScenario != nullptr && "Load arena scenario isn't found.");

    UGameInstance_Unknown* pGameInstance = UUnknownCommon::GetGameInstanceUnknown(this);

    TArray<FName> playerPartyMembers(pLoadArenaScenario->PlayerPartyMembers);
    if (pLoadArenaScenario->DontUseGMPlayerParty == false)
    {
        playerPartyMembers.Append(UUnknownCommon::GetGlobalMapPlayerController(this)->GetGMPawn()->GetPartyMembers());
    }
    FillPartyWithMembers(playerPartyMembers, pGameInstance->mPlayerPartyMembers);
    FillPartyWithMembers(pLoadArenaScenario->AIPartyMembers, pGameInstance->mAIPartyMembers);



    pGameInstance->mArenaResultScenario = pLoadArenaScenario->ArenaResultScenario;

    UUnknownCommon::GetGlobalMapGameMode(this)->StartArena(pLoadArenaScenario->ArenaLevelName);
}

void AActionScript_LoadArena::FillPartyWithMembers(const TArray<FName>& memberNames, TArray<TSubclassOf<ACreatureBase>>& partyMembers)
{
    UGameInstance_Unknown* pGameInstance = UUnknownCommon::GetGameInstanceUnknown(this);
    partyMembers.SetNum(0, false);
    for (auto& creatureName : memberNames)
    {
        FCreatureInfo* pCreatureInfo = pGameInstance->mpCreaturesInfoTable->FindRow<FCreatureInfo>(creatureName, "Can't find creature.");
#if !UE_BUILD_SHIPPING
        if (pCreatureInfo == nullptr)
        {
            UE_LOG(GlobalMapCommonLog, Error, TEXT("Creature %s doesn't exist."), *creatureName.ToString());
        }
#endif // #if !UE_BUILD_SHIPPING
        if (pCreatureInfo != nullptr)
        {
            partyMembers.Push(pCreatureInfo->mCreatureClass);
        }
    }
}
