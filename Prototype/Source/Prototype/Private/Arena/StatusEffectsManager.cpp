// Fill out your copyright notice in the Description page of Project Settings.


#include "Arena/StatusEffectsManager.h"
#include "Arena/ArenaUtilities.h"
#include "Arena/ArenaManager.h"
#include "Arena/Damageable.h"
#include "Arena/StatusEffect.h"
#include "Arena/TurnsManager.h"
#include "Arena/ArenaUnit_Interface.h"
#include "Arena/BattleLog/BattleLog.h"
#include "UnknownCommon.h"


UStatusEffectsManager::UStatusEffectsManager(const FObjectInitializer& objectInitializer)
    : Super(objectInitializer)
{
    PrimaryComponentTick.bCanEverTick = true;
    //bAutoActivate = true;
}

void UStatusEffectsManager::Activate(bool reset)
{
    Super::Activate(reset);

    //  ?
}

void UStatusEffectsManager::Deactivate()
{
    Super::Deactivate();

    for (AStatusEffect* pStatusEffect : mEffects)
    {
        pStatusEffect->Deactivate();
    }
}

void UStatusEffectsManager::PushStatusEffectTask(const FStatusEffectTask& task)
{
    if (IsActive() == false) return;

    if (mEffects.ContainsByPredicate([&task](const AStatusEffect* pStatusEffect) { return pStatusEffect->GetClass() == task.mEffectClass; }))
    {
        if (task.mPriority <= 0)
        {
            UE_LOG(ArenaCommonLog, Warning, TEXT("0 activation task priority for status effect %s"), *task.mEffectClass->GetName());
        }
        mStatusEffectTasks.Push(task);
    }
}

void UStatusEffectsManager::ExecuteStatusEffectTask(const FStatusEffectTask& task)
{
    if (IsActive() == false) return;

    AStatusEffect* pStatusEffect = GetEffectByType(task.mEffectClass);
    if (pStatusEffect != nullptr)
    {
        if (task.mDuration > 0)
        {
            pStatusEffect->Activate(task.mDuration, task.mStage);
            if (IsValid(task.mpResultBattleLogItem))
            {
                UBattleLogItem_Base* pBattleLogItem = task.mpResultBattleLogItem;
                pBattleLogItem->AddActivatedStatusEffect(task.mEffectClass, task.mDuration, task.mStage);
            }
        }
        else
        {
            pStatusEffect->Deactivate();
            if (IsValid(task.mpResultBattleLogItem))
            {
                UBattleLogItem_Base* pBattleLogItem = task.mpResultBattleLogItem;
                pBattleLogItem->AddDeactivatedStatusEffect(task.mEffectClass);
            }
        }
    }
}

void UStatusEffectsManager::ActivateStatusEffect(TSubclassOf<AStatusEffect> effectClass, int32 duration, int32 stage)
{
    if (IsActive() == false) return;

    AStatusEffect* pStatusEffect = GetEffectByType(effectClass);
    if (IsValid(pStatusEffect))
    {
        pStatusEffect->Activate(duration, stage);
    }
}

void UStatusEffectsManager::DeactivateStatusEffect(TSubclassOf<AStatusEffect> effectClass)
{
    if (IsActive() == false) return;

    AStatusEffect* pStatusEffect = GetEffectByType(effectClass);
    if (IsValid(pStatusEffect))
    {
        pStatusEffect->Deactivate();
    }
}

bool UStatusEffectsManager::IsStatusEffectActive(TSubclassOf<AStatusEffect> effectClass)
{
    AStatusEffect* pEffect = GetEffectByType(effectClass);
    return (pEffect != nullptr) && pEffect->IsActive();
}

void UStatusEffectsManager::BeginPlay()
{
    Super::BeginPlay();

    //

    UNKNOWN_NOT_ARENA_EDITOR_BEGIN(this)

        if (UArenaUtilities::AreSystemsSetup(this))
        {
            OnSystemsSetup();
        }
        else
        {
            UArenaUtilities::GetArenaManager(this)->mEventSystemsSetup.AddDynamic(this, &UStatusEffectsManager::OnSystemsSetup);
        }

    UNKNOWN_NOT_ARENA_EDITOR_END

    //	Construct supported effects:
    for (auto& effectClass : mSupportedStatusEffects)
    {
        AStatusEffect* pEffect = GetWorld()->SpawnActor<AStatusEffect>(effectClass.Get(),
                                                                       FTransform::Identity);
        pEffect->SetOwner(GetOwner());
        mEffects.Push(pEffect);
        pEffect->Setup(this, GetOwner());
        pEffect->mEventActivated.BindUObject(this, &UStatusEffectsManager::OnEffectActivated);
        pEffect->mEventDeactivated.BindUObject(this, &UStatusEffectsManager::OnEffectDeactivated);
    }
}

void UStatusEffectsManager::EndPlay(EEndPlayReason::Type reason)
{
    for (auto pEffect : mEffects)
    {
        pEffect->Destroy();
    }

    if (UArenaUtilities::AreSystemsSetup(this))
    {
        ATurnsManager* pTurnsManager = UArenaUtilities::GetTurnsManager(this);
        pTurnsManager->mEventFinalizeTurn.RemoveDynamic(this, &UStatusEffectsManager::OnFinalizeTurn);
        if (mDelegateApplyStatusEffectsBeforeActions.IsValid())
        {
            pTurnsManager->mEventApplyStatusEffectsBeforeActions.Remove(mDelegateApplyStatusEffectsBeforeActions); 
        }
        if (mDelegateApplyStatusEffectsAfterActions.IsValid())
        {
            pTurnsManager->mEventApplyStatusEffectsAfterActions.Remove(mDelegateApplyStatusEffectsAfterActions); 
        }
    }

    Super::EndPlay(reason);
}

void UStatusEffectsManager::OnSystemsSetup()
{
    ATurnsManager* pTurnsManager = UArenaUtilities::GetTurnsManager(this);
    pTurnsManager->mEventFinalizeTurn.AddUniqueDynamic(this, &UStatusEffectsManager::OnFinalizeTurn);
    mDelegateApplyStatusEffectsBeforeActions = pTurnsManager->mEventApplyStatusEffectsBeforeActions.AddUObject(this, &UStatusEffectsManager::ApplyStatusEffectsBeforeActions);
    mDelegateApplyStatusEffectsAfterActions = pTurnsManager->mEventApplyStatusEffectsAfterActions.AddUObject(this, &UStatusEffectsManager::ApplyStatusEffectsAfterActions);
}

void UStatusEffectsManager::ApplyStatusEffectsBeforeActions()
{
    for (AStatusEffect* effect : mEffects)
    {
        if (effect->IsActive())
        {
            ETimeOfApplication effectToA = effect->GetTimeOfApplication();
            if (effectToA == ETimeOfApplication::BeforeActions ||
                effectToA == ETimeOfApplication::BeforeAndAfterActions)
            {
                effect->Apply(ETimeOfApplication::BeforeActions);
            }
        }
    }
}

void UStatusEffectsManager::ApplyStatusEffectsAfterActions()
{
    for (AStatusEffect* effect : mEffects)
    {
        if (effect->IsActive())
        {
            ETimeOfApplication effectToA = effect->GetTimeOfApplication();
            if (effectToA == ETimeOfApplication::AfterActions ||
                effectToA == ETimeOfApplication::BeforeAndAfterActions)
            {
                effect->Apply(ETimeOfApplication::AfterActions);
            }
        }
    }
}

void UStatusEffectsManager::OnFinalizeTurn()
{
    for (AStatusEffect* statusEffect : mEffects)
    {
        statusEffect->OnTurnFinished();
    }

    //  It's not clear if OnTurnFinished should be called for newly activated status effects.
    ExecuteTasks();
}

void UStatusEffectsManager::OnEffectActivated(AStatusEffect* pStatusEffect)
{
    mEventStatusEffectActivated.Broadcast(GetOwner(), pStatusEffect);
}

void UStatusEffectsManager::OnEffectDeactivated(AStatusEffect* pStatusEffect)
{
    mEventStatusEffectDeactivated.Broadcast(GetOwner(), pStatusEffect);
}

const TArray<AStatusEffect*>& UStatusEffectsManager::GetAllEffects() const
{
    return mEffects;
}

AStatusEffect* UStatusEffectsManager::GetEffectByType(TSubclassOf<AStatusEffect> effectClass)
{
    auto CheckEffectClass = [&effectClass] (const AStatusEffect* pEffect) {
        return pEffect->GetClass() == effectClass;
    };
    AStatusEffect** dpEffect = mEffects.FindByPredicate(CheckEffectClass);
    return dpEffect == nullptr ? nullptr : *dpEffect;
}

TArray<AStatusEffect*> UStatusEffectsManager::GetActiveEffects()
{
    TArray<AStatusEffect*> activeEffects;
    for (auto pEffect : mEffects)
    {
        if (pEffect->IsActive())
        {
            activeEffects.Push(pEffect);
        }
    }
    return activeEffects;
}

void UStatusEffectsManager::StoreState_Inner(ArenaStateContext& context, State_StatusEffectsManager& state) const
{
    state.mStatusEffectsCount = mEffects.Num();
    state.mppStatusEffectsStates = mEffects.Num() > 0 ? context.AllocMemory<State_StatusEffect*>(mEffects.Num()) : nullptr;
    for (int32 i = 0; i < mEffects.Num(); ++i)
    {
        state.mppStatusEffectsStates[i] = mEffects[i]->StoreState(context);
    }
}

bool UStatusEffectsManager::RestoreState_Inner(const State_StatusEffectsManager& state)
{
    //  ~UGLY~ Assume that status effects have the same order:
    
    return true;
}

void UStatusEffectsManager::ExecuteTasks()
{
    if (mStatusEffectTasks.Num() > 0)
    {
        mStatusEffectTasks.Sort([](const FStatusEffectTask& left, const FStatusEffectTask& right) {
            return left.mPriority < right.mPriority;
        });

        //  Sort all tasks by a unique effect class:
        TMap<TSubclassOf<AStatusEffect>, TArray<int32>> tasksPerClass;
        for (int32 i = 0; i < mStatusEffectTasks.Num(); ++i)
        {
            FStatusEffectTask& task = mStatusEffectTasks[i];
            tasksPerClass.FindOrAdd(task.mEffectClass).Push(i);
        }

        for (auto effectClass_taskRefIndices : tasksPerClass)
        {
            TSubclassOf<AStatusEffect>& effectClass = effectClass_taskRefIndices.Key;
            TArray<int32>& taskRefIndices = effectClass_taskRefIndices.Value;

            AStatusEffect* pStatusEffect = GetEffectByType(effectClass);
            if (IsValid(pStatusEffect))
            {
                //  Counting that deactivation invalidates any previous activations,
                //  we should start tasks execution from the last found "Deactivation Task":

                int32 lastDeactivationTaskRefIndex = taskRefIndices.FindLastByPredicate([this](int32 taskIndex) {
                    return mStatusEffectTasks[taskIndex].mDuration <= 0;
                });

                //  If its not found - we start from the beginning:
                int32 taskToExecuteRefIndex = (lastDeactivationTaskRefIndex == INDEX_NONE) ? 0 : lastDeactivationTaskRefIndex;
                for (; taskToExecuteRefIndex < taskRefIndices.Num(); ++taskToExecuteRefIndex)
                {
                    int32 taskIndex = taskRefIndices[taskToExecuteRefIndex];
                    FStatusEffectTask& task = mStatusEffectTasks[taskIndex];
                    if (task.mDuration > 0)
                    {
                        pStatusEffect->Activate(task.mDuration, task.mStage);

                    }
                    else
                    {
                        pStatusEffect->Deactivate();
                    }
                }
            }
            else
            {
                UE_LOG(ArenaCommonLog, Error, TEXT("No status effect of class %s was found."), *(effectClass->GetName()));
            }
        }

        mStatusEffectTasks.Empty();
    }

    for (AStatusEffect* pStatusEffect : mEffects)
    {
        if (pStatusEffect->IsActive() && pStatusEffect->GetDuration() <= 0)
        {
            pStatusEffect->Deactivate();
        }
    }
}
