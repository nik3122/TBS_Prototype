#pragma once


#include "UnknownCommon.generated.h"


class UGameInstance_Unknown;
class UProgressManager;
class AGameMode_GlobalMap;
class APlayerController_GlobalMap;


#define UNKNOWN_ARENA_EDITOR

#ifdef UNKNOWN_ARENA_EDITOR
//  ~!~HACK~ Can't use IsArenaEditor as currently we have arena editor game mode from CustomEditors module (which is not runtime module).
//  ~TODO~ When arena editor game mode will be runtime capable this should be changed to IsArenaEditor() == false.
#define UNKNOWN_NOT_ARENA_EDITOR_BEGIN(worldContextObject)                       //if (UUnknownCommon::IsArena(worldContextObject)) {
#define UNKNOWN_NOT_ARENA_EDITOR_END                                             //}
#else // #ifdef UNKNOWN_ARENA_EDITOR
#define UNKNOWN_NOT_ARENA_EDITOR_BEGIN(worldContextObject)
#define UNKNOWN_NOT_ARENA_EDITOR_END
#endif // #ifdef UNKNOWN_ARENA_EDITOR


DECLARE_LOG_CATEGORY_EXTERN(UnknownCommonLog, All, All);

DECLARE_LOG_CATEGORY_EXTERN(GlobalMapCommonLog, All, All);


UCLASS()
class PROTOTYPE_API UUnknownCommon:
    public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:

    UFUNCTION(BlueprintPure, Category = "Unknown", meta = (WorldContext = "pWorldContextObject"))
    static UGameInstance_Unknown* GetGameInstanceUnknown(const UObject* pWorldContextObject);

    UFUNCTION(BlueprintPure, Category = "Unknown", meta = (WorldContext = "pWorldContextObject"))
    static UProgressManager* GetProgressManager(UObject* gWorldContextObject);

    UFUNCTION(BlueprintPure, Category = "Unknown|Arena", meta = (WorldContext = "pWorldContextObject"))
    static bool IsArenaEditor(const UObject* pWorldContextObject);

    UFUNCTION(BlueprintPure, Category = "Unknown|Arena", meta = (WorldContext = "pWorldContextObject"))
    static bool IsArena(const UObject* pWorldContextObject);

    //

    UFUNCTION(BlueprintPure, Category = "Unknown|GlobalMap", meta = (WorldContext = "pWorldContextObject", DeprecatedFunction))
    static AGameMode_GlobalMap* GetGlobalMapGameMode(const UObject* pWorldContextObject);

    UFUNCTION(BlueprintPure, Category = "Unknown|GlobalMap", meta = (WorldContext = "pWorldContextObject", DeprecatedFunction))
    static APlayerController_GlobalMap* GetGlobalMapPlayerController(const UObject* pWorldContextObject);

    UFUNCTION(BlueprintPure, Category = "Unknown|ArenaFight", meta = (DeterminesOutputType = "pClass"))
    static UObject* GetDefaultObject(UClass* pClass);
};