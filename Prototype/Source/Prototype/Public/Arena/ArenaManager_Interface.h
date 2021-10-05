#pragma once


//#include "CoreMinimal.h"
#include "ArenaManager_Interface.generated.h"


class ATurnsManager;
class APassMap;
class AFireSystem;
class AActionMap;
class AParty;
class AWeather;


UINTERFACE(Blueprintable)
class PROTOTYPE_API UArenaManager_Interface:
    public UInterface
{
    GENERATED_BODY()
};


class PROTOTYPE_API IArenaManager_Interface
{
    GENERATED_BODY()

public:

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Arena")
    AParty* GetPartyOne();

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Arena")
    AParty* GetPartyTwo();

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Arena")
    AParty* GetPlayerParty();

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Arena")
    AParty* GetAIParty();

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Arena")
    ATurnsManager* GetTurnsManager();

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Arena")
    APassMap* GetPassMap();

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Arena")
    AFireSystem* GetFireSystem();

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Arena")
    AInfectionSystem* GetInfectionSystem();

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Arena")
    AActionMap* GetActionMap();

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Arena")
    AWeather* GetWeather();

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Arena")
    bool AreSystemsSetup() const;
};