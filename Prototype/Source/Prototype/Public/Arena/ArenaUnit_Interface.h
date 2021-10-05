#pragma once


#include "ArenaUnit_Interface.generated.h"


class UDamageable;
class UActionManager;
class UTiledMovement;
class UStatusEffectsManager;
class UAnimationManager;
class UWeaponOperator;
class USocialComponent;
class UArenaUnitAttributes;


UINTERFACE(Blueprintable)
class PROTOTYPE_API UArenaUnit_Interface:
    public UInterface
{
    GENERATED_BODY()
};


class PROTOTYPE_API IArenaUnit_Interface
{
    GENERATED_BODY()

public:

    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Arena")
    UDamageable* GetDamageable();

    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Arena")
    UDamageable* GetVisualDamageable();

    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Arena")
    UActionManager* GetActionManager();

    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Arena")
    UTiledMovement* GetTiledMovement();

    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Arena")
    UStatusEffectsManager* GetStatusEffectsManager();

    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Arena")
    UAnimationManager* GetAnimationManager();

    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Arena")
    UWeaponOperator* GetWeaponOperator();

    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Arena")
    USocialComponent* GetSocialComponent();

    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Arena")
    UArenaUnitAttributes* GetArenaUnitAttributes();
};