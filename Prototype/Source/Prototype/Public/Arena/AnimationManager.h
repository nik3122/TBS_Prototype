#pragma once


#include "CoreMinimal.h"
#include "Animation/AnimationAsset.h"
#include "GenericAnimNotify.h"
#include "Weapon_Base.h"
#include "AnimationManager.generated.h"


USTRUCT(BlueprintType)
struct PROTOTYPE_API FSingleActionAnimation
{
    GENERATED_BODY()

public:

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    UAnimationAsset* mpAnimationAsset;
};


UENUM(BlueprintType)
enum class EBaseAnimState : uint8
{
    BaseAnimState_Default,
    BaseAnimState_Idle,
    BaseAnimState_Continuous,
    BaseAnimState_Combat,
    BaseAnimState_SpellCast,
};


UENUM(BlueprintType)
enum class EAttackType: uint8
{
    AttackType_1,
    AttackType_2,
    AttackType_3,
    AttackType_AOE,
    AttackType_Throw,
};


USTRUCT(BlueprintType)
struct FWeaponAnimState
{
    GENERATED_BODY()

public:

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unknown|Arena")
    EWeaponHandleType mWeaponHandleType;

};


UENUM(BlueprintType)
enum class ESpellCastType : uint8
{
    SpellCastAnimState_Mantra,
    SpellCastAnimState_Summon,
    SpellCastAnimState_RangeAttack,
    SpellCastAnimState_SetupStamp,
};


USTRUCT(BlueprintType)
struct FSpellCastAnimState
{
    GENERATED_BODY()

public:

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unknown|Arena")
    ESpellCastType mSpellCastType;
};


USTRUCT(BlueprintType)
struct FAnimState
{
    GENERATED_BODY()

public:

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unknown|Arena")
    EBaseAnimState mBaseAnimState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unknown|Arena")
    FWeaponAnimState mCombatAnimState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unknown|Arena")
    FSpellCastAnimState mSpellCastAnimState;


};


UCLASS(Blueprintable, ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class PROTOTYPE_API UAnimationManager:
    public UActorComponent
{
    GENERATED_BODY()

public:

    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FGenericAnimNotifyEvent, EGenericAnimNotify, genericAnimNotify);
    UPROPERTY(BlueprintAssignable, Category = "Unknown|Arena")
    FGenericAnimNotifyEvent mGenericAnimNotifyEvent;

    DECLARE_DYNAMIC_MULTICAST_DELEGATE(FEventSingleActionAnimationFinished);
    UPROPERTY(BlueprintAssignable, Category = "Unknown|Arena")
    FEventSingleActionAnimationFinished mEventSingleActionAnimationFinished;

    DECLARE_DYNAMIC_DELEGATE(FDelegateSingleActionAnimationFinished);

    UPROPERTY(BlueprintReadWrite, Category = "Unknown|Arena")
    FAnimState mAnimState;

public:

    UAnimationManager();

    void OnGenericAnimNotify(EGenericAnimNotify genericAnimNotify);

    UFUNCTION(BlueprintPure, Category = "Unknown|Arena")
    USkeletalMeshComponent* GetSkeletalMeshComponent();

    UFUNCTION(BlueprintPure, Category = "Unknown|Arena")
    UAnimMontage* GetSingleActionsAnimMontage() const;

    //  Returns true if anim montage started playing single action section.
    UFUNCTION(BlueprintCallable, Category = "Unknown|Arena")
    bool TryPlaySingleActionAnimation(const FName& singleActionSection);

    UFUNCTION(BlueprintCallable, Category = "Unknown|Arena")
    bool TryPlaySingleActionAnimationWithCallback(const FName& singleActionSection,
                                                  const FDelegateSingleActionAnimationFinished& onFinishedCallback);

    void StopAnyAnimationAndSwitchToIdle();

protected:

    void InitializeComponent() override;

    void BeginPlay() override;

    UFUNCTION()
    void OnMontageEnded(UAnimMontage* pMontage, bool isInterrupted);

private:

    UPROPERTY(EditAnywhere, Category = "Unknown|Arena")
    USkeletalMeshComponent* mpCreatureSkeletalMesh;

    UPROPERTY(EditAnywhere, Category = "Unknown|Arena")
    UAnimMontage* mpSingleActionsMontage;

    FDelegateSingleActionAnimationFinished mOnSingleActionAnimationFinishedCallback;
};