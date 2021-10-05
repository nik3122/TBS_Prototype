

#include "Arena/AnimationManager.h"
#include "Arena/ArenaUtilities.h"


UAnimationManager::UAnimationManager()
{
    PrimaryComponentTick.bCanEverTick = false;
    bWantsInitializeComponent = true;
}

void UAnimationManager::OnGenericAnimNotify(EGenericAnimNotify genericAnimNotify)
{
    //	If the single animation finished switch to default anim state:
    if (genericAnimNotify == EGenericAnimNotify::EGenericAnimNotify_SingleActionAnimFinished)
    {
        mAnimState.mBaseAnimState = EBaseAnimState::BaseAnimState_Default;
    }
    mGenericAnimNotifyEvent.Broadcast(genericAnimNotify);
}

USkeletalMeshComponent* UAnimationManager::GetSkeletalMeshComponent()
{
    return mpCreatureSkeletalMesh;
}

UAnimMontage* UAnimationManager::GetSingleActionsAnimMontage() const
{
    return mpSingleActionsMontage;
}

void UAnimationManager::InitializeComponent()
{
    Super::InitializeComponent();

    //  Skip auto-detection if the reference already set.
    if (mpCreatureSkeletalMesh == nullptr)
    {
        mpCreatureSkeletalMesh = GetOwner()->FindComponentByClass<USkeletalMeshComponent>();
    }
    check(mpCreatureSkeletalMesh != nullptr && "No skeletal mesh component was found by UAnimationManager.");
}

void UAnimationManager::BeginPlay()
{
    Super::BeginPlay();

    if (mpCreatureSkeletalMesh == nullptr)
    {
        UE_LOG(ArenaCommonLog, Error, TEXT("UAnimationManager::mpCreatureSkeletalMesh must be set to the creature skeletal mesh."));
    }

    UAnimInstance* pAnimInstance = mpCreatureSkeletalMesh->GetAnimInstance();
    pAnimInstance->OnMontageEnded.AddDynamic(this, &UAnimationManager::OnMontageEnded);
}

void UAnimationManager::StopAnyAnimationAndSwitchToIdle()
{
    //  ~TODO~
}

bool UAnimationManager::TryPlaySingleActionAnimation(const FName& singleActionSection)
{
    if (mpSingleActionsMontage->IsValidSectionName(singleActionSection))
    {
        UAnimInstance* pAnimInstance = mpCreatureSkeletalMesh->GetAnimInstance();
        if (pAnimInstance != nullptr)
        {
            float montageLength = pAnimInstance->Montage_Play(mpSingleActionsMontage);
            if (montageLength > 0.f)
            {
                pAnimInstance->Montage_JumpToSection(singleActionSection, mpSingleActionsMontage);
                return true;
            }
        }
    }
    return false;
}

bool UAnimationManager::TryPlaySingleActionAnimationWithCallback(const FName& singleActionSection,
                                                                 const FDelegateSingleActionAnimationFinished& onFinishedCallback)
{
    bool isPlaying = TryPlaySingleActionAnimation(singleActionSection);
    if (isPlaying)
    {
        mOnSingleActionAnimationFinishedCallback = onFinishedCallback;
    }
    return isPlaying;
}

void UAnimationManager::OnMontageEnded(UAnimMontage* pMontage, bool isInterrupted)
{
    if (pMontage == mpSingleActionsMontage)
    {
        mOnSingleActionAnimationFinishedCallback.ExecuteIfBound();
        mOnSingleActionAnimationFinishedCallback.Clear();
        mEventSingleActionAnimationFinished.Broadcast();
    }
}
