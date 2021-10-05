// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Arena/ArenaUnit_Interface.h"
#include "Arena/ArenaState.h"
#include "Engine/DataTable.h"
#include "Math/Color.h"
#include "Version.h"
#include "CreatureBase.generated.h"


class ACreatureGhost;
class UTiledMovement;
class UDamageable;
class UActionManager;
class UStatusEffectsManager;
class UAnimationManager;
class UWeaponOperator;
class USocialComponent;
class UArenaUnitAttributes;

struct State_Damageable;
struct State_TiledMovement;
struct State_StatusEffectsManager;
struct State_ActionManager;


struct State_CreatureBase
    : public State_Base
{
    DECLARE_CLASS_VERSION(1, 0);
    const ANSICHAR* mpClassName;
    int32 mClassNameSize;
    State_Damageable* mpDamageableState;
    State_TiledMovement* mpTiledMovementState;
    State_StatusEffectsManager* mpStatusEffectsManagerState;
    State_ActionManager* mpActionManagerState;
};


UENUM(BlueprintType)
enum class ECreatureClass : uint8
{
    CreatureClass_Wizard,
    CreatureClass_Warrior,
    CreatureClass_MAX,
};


UCLASS()
class PROTOTYPE_API ACreatureBase
    : public APawn
    , public IArenaUnit_Interface
    , public ArenaStateObject_Base<State_CreatureBase>
{
    CHECK_CLASS_VERSION_EQUAL(State_CreatureBase, 1, 0);

    GENERATED_BODY()

public:

    ACreatureBase(const FObjectInitializer& objectInitializer = FObjectInitializer::Get());

    void PostInitializeComponents() override;

    UFUNCTION(BlueprintPure, Category = "Arena")
    UTexture2D* GetCreatureImage() const;

    void SetPartyMemberColor(const FLinearColor& color);

    UFUNCTION(BlueprintPure, Category = "Arena")
    const FLinearColor& GetPartyMemberColor() const;

    UFUNCTION(BlueprintPure, Category = "Arena")
    ACreatureGhost* GetCreatureGhost() const;

    UFUNCTION(BlueprintCallable, Category = "Arena")
    void SetStunned(bool isStunned);

    UFUNCTION(BlueprintPure, Category = "Arena")
    bool IsStunned() const;
    
    UFUNCTION(BlueprintPure, Category = "Arena")
    UTiledMovement* GetTiledMovementComponent();
    
    UFUNCTION(BlueprintPure, Category = "Arena")
    UDamageable* GetDamageableComponent();

    UFUNCTION(BlueprintPure, Category = "Arena")
    UActionManager* GetActionManagerComponent();

    UFUNCTION(BlueprintPure, Category = "Arena")
    UStatusEffectsManager* GetStatusEffectsManagerComponent();

    UFUNCTION(BlueprintPure, Category = "Arena")
    UArenaUnitAttributes* GetArenaUnitAttributesComponent() const;

    UDamageable* GetDamageable() const;

    UDamageable* GetVisualDamageable() const;

    UActionManager* GetActionManager() const;

    UTiledMovement* GetTiledMovement() const;

    UStatusEffectsManager* GetStatusEffectsManager() const;

    UAnimationManager* GetAnimationManager() const;

    UWeaponOperator* GetWeaponOperator() const;

    USocialComponent* GetSocialComponent() const;

    UArenaUnitAttributes* GetArenaUnitAttributes() const;

    void SetIsFocusedPartyMember(bool isFocused);

    //	ArenaUnit_Interface::

    UDamageable* GetDamageable_Implementation() override;

    UDamageable* GetVisualDamageable_Implementation() override;

    UActionManager* GetActionManager_Implementation() override;

    UTiledMovement* GetTiledMovement_Implementation() override;

    UStatusEffectsManager* GetStatusEffectsManager_Implementation() override;

    UAnimationManager* GetAnimationManager_Implementation() override;

    UWeaponOperator* GetWeaponOperator_Implementation() override;

    USocialComponent* GetSocialComponent_Implementation() override;

    UArenaUnitAttributes* GetArenaUnitAttributes_Implementation() override;

protected:

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Arena")
    ECreatureClass mCreatureClass;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Arena")
    USkeletalMeshComponent* mpSkeletalMesh = nullptr;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Arena")
    UTiledMovement* mpMovement = nullptr;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Arena")
    UDamageable* mpDamageable = nullptr;

    UPROPERTY(BlueprintReadOnly, Category = "Arena|Creature")
    UDamageable* mVisualDamageable = nullptr;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Arena")
    UActionManager* mpActionManager = nullptr;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Arena")
    UStatusEffectsManager* mpStatusEffectsManager = nullptr;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Arena")
    UAnimationManager* mpAnimationManager = nullptr;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Arena")
    UWeaponOperator* mpWeaponOperator = nullptr;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Arena")
    USocialComponent* mpSocialComponent = nullptr;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Arena")
    UArenaUnitAttributes* mArenaUnitAttributes = nullptr;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Arena")
    UTexture2D* mpCreatureImage;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Arena")
    TSubclassOf<ACreatureGhost> mCreatureGhostClass;

protected:

    void BeginPlay() override;

    UFUNCTION(BlueprintImplementableEvent, Meta = (DisplayName = ""))
    void K2_OnSetIsFocusedPartyMember(bool isFocused);

    UFUNCTION(BlueprintImplementableEvent, Meta = (DisplayName = "OnPartyMemberColorChanged"))
    void K2_OnPartyMemberColorChanged();
    
    UFUNCTION()
    void OnSystemsSetup();

    UFUNCTION()
    void OnFinalizeTurn();

    void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    UFUNCTION()
    void OnDeath(UDamageable* pDamageable);

    UFUNCTION()
    void OnRevive(UDamageable* pDamageable);

    UFUNCTION()
    void OnVisualDeath(UDamageable* visualDamageable);

    UFUNCTION()
    void OnVisualRevive(UDamageable* visualDamageable);

    void StoreState_Inner(ArenaStateContext& context, State_CreatureBase& state) const override;

    bool RestoreState_Inner(const State_CreatureBase& state) override;
    
private:

    UPROPERTY(ReplicatedUsing = OnRep_PartyMemberColor)
    FLinearColor mPartyMemberColor;

    UPROPERTY()
    ACreatureGhost* mCreatureGhost = nullptr;

    bool mIsStunned = false;

    bool mIsFocusedPartyMember = false;

private:

    void SpawnCreatureGhost();

    UFUNCTION()
    void OnRep_PartyMemberColor();
};


USTRUCT(BlueprintType)
struct FCreatureInfo
    : public FTableRowBase
{
    GENERATED_BODY()

public:

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TSubclassOf<ACreatureBase> mCreatureClass;
};
