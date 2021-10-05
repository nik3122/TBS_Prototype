#pragma once


#include "CoreMinimal.h"
#include "Arena/StatusEffectsManager.h"
#include "GameFramework/Info.h"
#include "InfectionSystem.generated.h"


class APassMap;
class UStatusEffectsManager;
class AStatusEffect_Infection;
class UNiagaraSystem;
class UNiagaraComponent;
class UTiledMovement;


USTRUCT()
struct FInfectableInfo
{
    GENERATED_BODY()

public:

    UPROPERTY()
    AStatusEffect_Infection* mpStatusEffect = nullptr;

    UPROPERTY()
    UTiledMovement* mpPassMapMovement = nullptr;

public:

    bool operator==(const FInfectableInfo& right) const
    {
        return mpStatusEffect == right.mpStatusEffect && mpPassMapMovement == right.mpPassMapMovement;
    }
};


USTRUCT()
struct FInfectedTile
{
    GENERATED_BODY()

public:

    UPROPERTY(EditAnywhere, Category = "Arena")
    FIntPoint mTile;

    UPROPERTY(EditAnywhere, Category = "Arena")
    int16 mDuration = 0;

    UPROPERTY()
    UNiagaraComponent* mpSpawnedNiagara = nullptr;

    bool mIsSetManually = false;

    bool mIsGenerated = false;
};


UCLASS(Blueprintable)
class AInfectionSystem
    : public AInfo
{
    GENERATED_BODY()

public:

    void SetupSystem(APassMap* pPassMap);

    void RegisterInfectable(AStatusEffect_Infection* pStatusEffect);

    void UnregisterInfectable(AStatusEffect_Infection* pStatusEffect);

    //  If duration == 0 - sets tile uninfected.
    //  If duration == -1 - sets tile infinitely infected.
    //  If duration > 0 - sets tile infected for a number of turns.
    UFUNCTION(BlueprintCallable, Category = "Arena")
    void SetTileInfected(const FIntPoint& tile, int32 duration);

    void OnFinalizeTurn();

#if WITH_EDITOR
    void PostEditChangeProperty(FPropertyChangedEvent& propertyChangedEvent) override;
#endif // #if WITH_EDITOR

protected:

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Arena")
    UNiagaraSystem* mpInfectedTilesNiagara = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Arena")
    FStatusEffectTask mFirstTimeInfectedStatusEffectTask;

    UPROPERTY()
    TArray<FInfectableInfo> mRegisteredInfectables;

    UPROPERTY(EditAnywhere, Category = "Arena")
    TArray<FInfectedTile> mInfectedTiles;

    //  Stores infection power for each tile. Infection power equals the number of infection sources influencing this tile.
    //  Can be used to determine either the stage or duration of the applied infection status effect.
    TArray<uint8> mInfectionMap;

private:

    void RegisterInfectableInner(AStatusEffect_Infection* pStatusEffect, UTiledMovement* pPassMapMovement);

    void SpawnVisuals(FInfectedTile& infectedTile, APassMap* pPassMap);
};