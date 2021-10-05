
#include "UnknownCommon.h"

#include "GameInstance_Unknown.h"
#include "GameMode_GlobalMap.h"
#include "PlayerController_GlobalMap.h"
#include "ArenaEditor/GameMode_ArenaEditor.h"
#include "Arena/GameMode_ArenaFight.h"
#include "Kismet/GameplayStatics.h"


DEFINE_LOG_CATEGORY(UnknownCommonLog)

DEFINE_LOG_CATEGORY(GlobalMapCommonLog)


#if !UE_BUILD_SHIPPING
#pragma optimize("", off)
#endif // #if !UE_BUILD_SHIPPING


UGameInstance_Unknown* UUnknownCommon::GetGameInstanceUnknown(const UObject* pWorldContextObject)
{
    auto pGameInstance = Cast<UGameInstance_Unknown>(UGameplayStatics::GetGameInstance(pWorldContextObject));
    check(IsValid(pGameInstance) && "Game instance should be GameInstance_Unknown!");
    return pGameInstance;
}

UProgressManager* UUnknownCommon::GetProgressManager(UObject* gWorldContextObject)
{
    return GetGameInstanceUnknown(gWorldContextObject)->GetProgressManager();
}

bool UUnknownCommon::IsArenaEditor(const UObject* pWorldContextObject)
{
    auto pGameMode = Cast<AGameMode_ArenaEditor>(UGameplayStatics::GetGameMode(pWorldContextObject));
    return pGameMode != nullptr;
}

bool UUnknownCommon::IsArena(const UObject* pWorldContextObject)
{
    auto pGameMode = Cast<AGameMode_ArenaFight>(UGameplayStatics::GetGameMode(pWorldContextObject));
    return pGameMode != nullptr;
}

AGameMode_GlobalMap* UUnknownCommon::GetGlobalMapGameMode(const UObject* pWorldContextObject)
{
    auto pGameMode_GlobalMap = Cast<AGameMode_GlobalMap>(UGameplayStatics::GetGameMode(pWorldContextObject));
    check(IsValid(pGameMode_GlobalMap) && "Current game mode is not \"AGameMode_GlobalMap\".");
    return pGameMode_GlobalMap;
}

APlayerController_GlobalMap* UUnknownCommon::GetGlobalMapPlayerController(const UObject* pWorldContextObject)
{
    //  ~!~ ~SINGLE~ SINGLE PLAYER ONLY solution (player index = 0):
    auto pGlobalMapPlayerController = Cast<APlayerController_GlobalMap>(UGameplayStatics::GetPlayerController(pWorldContextObject, 0));
    check(IsValid(pGlobalMapPlayerController) && "Current player controller is not \"APlayerCOntroller_GlobalMap\".");
    return pGlobalMapPlayerController;
}

UObject* UUnknownCommon::GetDefaultObject(UClass* pClass)
{
    return pClass->GetDefaultObject();
}


#if !UE_BUILD_SHIPPING
#pragma optimize("", on)
#endif // #if !UE_BUILD_SHIPPING
