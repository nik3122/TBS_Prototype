

#include "Arena/ArenaUtilities.h"
#include "Arena/PlayerController_ArenaFight.h"
#include "Arena/ArenaManager_Interface.h"
#include "Arena/ArenaManager.h"
#include "Arena/PassMap.h"
#include "Arena/ActionMap.h"
#include "Arena/TurnsManager.h"
#include "Arena/FireSystem.h"
#include "Arena/TrapSystem.h"
#include "Arena/InfectionSystem.h"
#include "Arena/BattleLog/BattleLog.h"
#include "Arena/GameState_Arena.h"
#include "EngineUtils.h"
#include "GameInstance_Unknown.h"
#include "Kismet/GameplayStatics.h"


DEFINE_LOG_CATEGORY(ArenaCommonLog)

DEFINE_LOG_CATEGORY(ArenaTurnsFlowLog)

DEFINE_LOG_CATEGORY(ArenaActionsLog)

DEFINE_LOG_CATEGORY(ArenaAILog)


#if !UE_BUILD_SHIPPING
#pragma optimize("", off)
#endif // #if !UE_BUILD_SHIPPING


bool UArenaUtilities::AreSystemsSetup(const UObject* pWorldContextObject)
{
    //  ~!~?~ Try* is used for cases something tries to call AreSystemsSetup in EndPlay. During PIE shutdown this trigger assertion as the GameState doesn't exist anymore.
    AArenaManager* pArenaManager = GetArenaManager(pWorldContextObject);
    return IsValid(pArenaManager) ? pArenaManager->AreSystemsSetup_Implementation() : false;
}

void UArenaUtilities::ExecuteWhenSystemsSetup(const UObject* pWorldContextObject,
                                              const FOnSystemsSetupDelegate& onSystemsSetupDelegate)
{
    if (AreSystemsSetup(pWorldContextObject))
    {
        onSystemsSetupDelegate.ExecuteIfBound();
    }
    else
    {
        AArenaManager* pArenaManger = GetArenaManager(pWorldContextObject);
        pArenaManger->mEventSystemsSetup.Add(onSystemsSetupDelegate);
    }
}

void UArenaUtilities::GetArenaManagerInterface(const UObject* pWorldContextObject,
                                               TScriptInterface<IArenaManager_Interface>& arenaManagerInterface)
{
    UObject* pArenaManagerObject = GetArenaManagerObject(pWorldContextObject);
    arenaManagerInterface.SetObject(pArenaManagerObject);
}

AArenaManager* UArenaUtilities::GetArenaManager(const UObject* pWorldContextObject)
{
    auto pArenaGameState = Cast<AGameState_Arena>(UGameplayStatics::GetGameState(pWorldContextObject));
    if (IsValid(pArenaGameState))
    {
        if (auto pArenaManager = pArenaGameState->GetArenaManager())
        {
            return pArenaManager;
        }
    }
    return *TActorIterator<AArenaManager>(pWorldContextObject->GetWorld());
}

AGameState_Arena* UArenaUtilities::GetArenaGameState(const UObject* pWorldContextObject)
{
    auto pArenaGameState = Cast<AGameState_Arena>(UGameplayStatics::GetGameState(pWorldContextObject));
    if(!IsValid(pArenaGameState))
    {
        UE_LOG(ArenaCommonLog, Fatal, TEXT("ArenaGameState is NULL"));
    }
    return pArenaGameState;
}

ATurnsManager* UArenaUtilities::GetTurnsManager(const UObject* pWorldContextObject)
{
    UObject* pArenaManagerObject = GetArenaManagerObject(pWorldContextObject);
    auto pTurnsManager = IArenaManager_Interface::Execute_GetTurnsManager(pArenaManagerObject);
    if(!IsValid(pTurnsManager))
    {
        UE_LOG(ArenaCommonLog, Fatal, TEXT("TurnsManager is NULL"));
    }
    return pTurnsManager;
}

APassMap* UArenaUtilities::GetPassMap(const UObject* pWorldContextObject)
{
    UObject* pArenaManagerObject = GetArenaManagerObject(pWorldContextObject);
    auto pPassMap = IArenaManager_Interface::Execute_GetPassMap(pArenaManagerObject);
    if(!IsValid(pPassMap))
    {
        UE_LOG(ArenaCommonLog, Fatal, TEXT("PassMap is NULL"));
    }
    return pPassMap;
}

AFireSystem* UArenaUtilities::GetFireSystem(const UObject* pWorldContextObject)
{
    UObject* pArenaManagerObject = GetArenaManagerObject(pWorldContextObject);
    auto pFireSystem = IArenaManager_Interface::Execute_GetFireSystem(pArenaManagerObject);
    if(!IsValid(pFireSystem))
    {
        UE_LOG(ArenaCommonLog, Fatal, TEXT("FireSystem is NULL"));
    }
    return pFireSystem;
}

AActionMap* UArenaUtilities::GetActionMap(const UObject* pWorldContextObject)
{
    UObject* pArenaManagerObject = GetArenaManagerObject(pWorldContextObject);
    auto pActionMap = IArenaManager_Interface::Execute_GetActionMap(pArenaManagerObject);
    if(!IsValid(pActionMap))
    {
        UE_LOG(ArenaCommonLog, Fatal, TEXT("ActionMap is NULL"));
    }
    return pActionMap;
}

UBattleLog* UArenaUtilities::GetBattleLog(const UObject* pWorldContextObject)
{
    UBattleLog* pBattleLog = GetActionMap(pWorldContextObject)->GetBattleLog();
    if(!IsValid(pBattleLog))
    {
        UE_LOG(ArenaCommonLog, Fatal, TEXT("BattleLog is NULL"));
    }
    return pBattleLog;
}

UTrapSystem* UArenaUtilities::GetTrapSystem(const UObject* pWorldContextObject)
{
    UTrapSystem* pTrapSystem = GetPassMap(pWorldContextObject)->GetTrapSystem();
    if(!IsValid(pTrapSystem))
    {
        UE_LOG(ArenaCommonLog, Fatal, TEXT("TrapSystem is NULL"));
    }
    return pTrapSystem;
}

AInfectionSystem* UArenaUtilities::GetInfectionSystem(const UObject* pWorldContextObject)
{
    UObject* pArenaManagerObject = GetArenaManagerObject(pWorldContextObject);
    auto pInfectionSystem = IArenaManager_Interface::Execute_GetInfectionSystem(pArenaManagerObject);
    if(!IsValid(pInfectionSystem))
    {
        UE_LOG(ArenaCommonLog, Fatal, TEXT("InfectionSystem is NULL"));
    }
    return pInfectionSystem;
}

APlayerController_ArenaFight* UArenaUtilities::GetArenaFightPlayerController(const UObject* pWorldContextObject)
{
    auto pArenaFightPlayerController = TryGetArenaFightPlayerController(pWorldContextObject);
    if(!IsValid(pArenaFightPlayerController))
    {
        UE_LOG(ArenaCommonLog, Fatal, TEXT("Current player controller is not \"APlayerController_ArenaFight\""));
    }
    return pArenaFightPlayerController;
}

APlayerController_ArenaFight* UArenaUtilities::TryGetArenaFightPlayerController(const UObject* pWorldContextObject)
{
    return Cast<APlayerController_ArenaFight>(UGameplayStatics::GetPlayerController(pWorldContextObject, 0));
}

UObject* UArenaUtilities::GetArenaManagerObject(const UObject* pWorldContextObject)
{
    UObject* pArenaManagerObject = GetArenaManager(pWorldContextObject);
    if(IsValid(pArenaManagerObject) == false ||
       pArenaManagerObject->Implements<UArenaManager_Interface>() == false)
    {
        UE_LOG(ArenaCommonLog, Fatal, TEXT("ArenaManagerObject is NULL"));
    }
    return pArenaManagerObject;
}

#if !UE_BUILD_SHIPPING
#pragma optimize("", on)
#endif // #if !UE_BUILD_SHIPPING