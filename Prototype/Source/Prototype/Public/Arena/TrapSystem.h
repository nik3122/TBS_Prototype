#pragma once


#include "CoreMinimal.h"
#include "Trap_Base.h"
#include "Components/ActorComponent.h"
#include "TrapSystem.generated.h"


class APassMap;


UCLASS()
class UTrapSystem
    : public UActorComponent
{
    GENERATED_BODY()

public:

    void SetupSystem(APassMap* pPassMap);

    void RegisterTrap(ATrap_Base* pTrap);

    //  ~!~HACK~UGLY~ Its used by movement actions on PreExecutionApply to trigger traps.
    //  It is crucial for actions to call this in order of execution to work narratively.
    void ProcessMovement(const TArray<FIntPoint>& path, ACreatureBase* pMovingCreature);

    UFUNCTION(BlueprintCallable, Category = "Unknown|Arena")
    ATrap_Base* TrySpawnTrap(TSubclassOf<ATrap_Base> trapClass, const FIntPoint& tile);

    UFUNCTION(BlueprintCallable, Category = "Unknown|Arena")
    ATrap_Base* GetTrapAtTile(const FIntPoint& tile);

    UFUNCTION(BlueprintCallable, Category = "Unknown|Arena")
    void RemoveTrapAtTile(const FIntPoint& tile);

protected:

    APassMap* mpPassMap;

    UPROPERTY()
    TArray<ATrap_Base*> mTriggeredTraps;

    TArray<ATrap_Base*> mRegisteredTraps;

protected:

    UFUNCTION()
    void OnSystemsSetup();

    UFUNCTION()
    void OnFinalizeTurn();

public:

    void SetupTrap(ATrap_Base* pTrap);
};
