// Fill out your copyright notice in the Description page of Project Settings.


#include "Arena/StatusEffect.h"
#include "Arena/ArenaUtilities.h"


AStatusEffect::AStatusEffect(const FObjectInitializer& objectInitializer)
    : Super(objectInitializer)
    , mpIcon(nullptr)
    , mpTargetActor(nullptr)
    , mTurnsTimer(0)
    , mStage(0)
{
    PrimaryActorTick.bCanEverTick = false;
}

void AStatusEffect::Setup(UStatusEffectsManager* pStatusEffectsManager, AActor* pTargetActor)
{
    check(pTargetActor != nullptr && "AStatusEffect::Setup(): pTargetActor is invalid.");
    mpTargetActor = pTargetActor;
    mpStatusEffectManager = pStatusEffectsManager;
    AttachToActor(mpTargetActor, FAttachmentTransformRules::SnapToTargetIncludingScale);
    Setup_Inner();
}

void AStatusEffect::Apply(ETimeOfApplication timeOfApplication)
{
    if (IsActive() == false)
    {
        UE_LOG(ArenaCommonLog, Warning, TEXT("Trying to apply innactive status effect %s."),
               *GetName());
    }

    Apply_Inner(timeOfApplication);
}

UTexture2D* AStatusEffect::GetIcon() const
{
    return mpIcon;
}

ETimeOfApplication AStatusEffect::GetTimeOfApplication() const
{
    return mTimeOfApplication;
}

void AStatusEffect::Activate(int32 duration,
                             int32 stage)
{
    if (IsActive())
    {
        Reactivate_Inner(duration,
                         stage,
                         mTurnsTimer,
                         mStage);
    }
    else
    {
        mTurnsTimer = duration;
        mStage = stage;
        Activate_Inner(stage);
    }
}

void AStatusEffect::Deactivate()
{
    if (IsActive())
    {
        Deactivate_Inner();
    }
    mTurnsTimer = 0;
    mStage = 0;
}

void AStatusEffect::OnTurnFinished()
{
    if (IsActive())
    {
        --mTurnsTimer;
    }
}

UStatusEffectsManager* AStatusEffect::GetStatusEffectManager() const
{
    return mpStatusEffectManager;
}

bool AStatusEffect::IsActive() const
{
    return mTurnsTimer > 0;
}

int32 AStatusEffect::GetDuration() const
{
    return mTurnsTimer;
}

int32 AStatusEffect::GetStage() const
{
    return mStage;
}

void AStatusEffect::StoreState_Inner(ArenaStateContext& context, State_StatusEffect& state) const
{
    auto className = StringCast<ANSICHAR>(*GetClass()->GetName());
    //  ~!~ Somehow last (null) char is not included into conversion:
    state.mpClassName = context.AllocText(className.Get(), className.Length() + 1);
    state.mClassNameSize = className.Length();
    state.mTurnsTimer = mTurnsTimer;
}

bool AStatusEffect::RestoreState_Inner(const State_StatusEffect& state)
{
    return true;
}

const FText& AStatusEffect::GetEffectName() const
{
    return mName;
}

const FText& AStatusEffect::GetFullDescription() const
{
    return mFullDescription;
}
