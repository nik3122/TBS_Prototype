#pragma once


#include "ArenaUtilities.generated.h"


DECLARE_LOG_CATEGORY_EXTERN(ArenaCommonLog, All, All);

DECLARE_LOG_CATEGORY_EXTERN(ArenaTurnsFlowLog, All, All);

DECLARE_LOG_CATEGORY_EXTERN(ArenaActionsLog, All, All);

DECLARE_LOG_CATEGORY_EXTERN(ArenaAILog, All, All);


class UGameInstance_Unknown;
class IArenaManager_Interface;
class AArenaManager;
class ATurnsManager;
class APassMap;
class AFireSystem;
class AGameMode_ArenaFight;
class AActionMap;
class ATurnBasedAction;
class APlayerController_ArenaFight;
class UBattleLog;
class UTrapSystem;
class AGameState_Arena;
class AInfectionSystem;


DECLARE_DYNAMIC_DELEGATE(FOnSystemsSetupDelegate);


UCLASS()
class UArenaUtilities
    : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:

    UFUNCTION(BlueprintPure, Category = "Arena", meta = (WorldContext = "pWorldContextObject"))
    static bool AreSystemsSetup(const UObject* pWorldContextObject);

    UFUNCTION(BlueprintCallable, Category = "Arena", meta = (WorldContext = "pWorldContextObject"))
    static void ExecuteWhenSystemsSetup(const UObject* pWorldContextObject,
                                        const FOnSystemsSetupDelegate& onSystemsSetupDelegate);

    UFUNCTION(BlueprintPure, Category = "Arena", meta = (WorldContext = "pWorldContextObject"))
    static void GetArenaManagerInterface(const UObject* pWorldContextObject,
                                         TScriptInterface<IArenaManager_Interface>& arenaManagerInterface);

    UFUNCTION(BlueprintPure, Category = "Arena", meta = (WorldContext = "pWorldContextObject"))
    static AArenaManager* GetArenaManager(const UObject* pWorldContextObject);

    UFUNCTION(BlueprintPure, Category = "Arena", meta = (WorldContext = "pWorldContextObject"))
    static AGameState_Arena* GetArenaGameState(const UObject* pWorldContextObject);

    UFUNCTION(BlueprintPure, Category = "Arena", meta = (WorldContext = "pWorldContextObject"))
    static ATurnsManager* GetTurnsManager(const UObject* pWorldContextObject);

    UFUNCTION(BlueprintPure, Category = "Arena", meta = (WorldContext = "pWorldContextObject"))
    static APassMap* GetPassMap(const UObject* pWorldContextObject);

    UFUNCTION(BlueprintPure, Category = "Arena", meta = (WorldContext = "pWorldContextObject"))
    static AFireSystem* GetFireSystem(const UObject* pWorldContextObject);

    UFUNCTION(BlueprintPure, Category = "Arena", meta = (WorldContext = "pWorldContextObject"))
    static AActionMap* GetActionMap(const UObject* pWorldContextObject);

    UFUNCTION(BlueprintPure, Category = "Arena", meta = (WorldContext = "pWorldContextObject"))
    static UBattleLog* GetBattleLog(const UObject* pWorldContextObject);

    UFUNCTION(BlueprintPure, Category = "Arena", meta = (WorldContext = "pWorldContextObject"))
    static UTrapSystem* GetTrapSystem(const UObject* pWorldContextObject);

    UFUNCTION(BlueprintPure, Category = "Arena", meta = (WorldContext = "pWorldContextObject"))
    static AInfectionSystem* GetInfectionSystem(const UObject* pWorldContextObject);

    UFUNCTION(BlueprintPure, Category = "Arena", meta = (WorldContext = "pWorldContextObject"))
    static APlayerController_ArenaFight* GetArenaFightPlayerController(const UObject* pWorldContextObject);

    UFUNCTION(BlueprintPure, Category = "Arena", meta = (WorldContext = "pWorldContextObject"))
    static APlayerController_ArenaFight* TryGetArenaFightPlayerController(const UObject* pWorldContextObject);

private:

    static UObject* GetArenaManagerObject(const UObject* pWorldContextObject);
};