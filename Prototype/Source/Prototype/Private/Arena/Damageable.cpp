// Fill out your copyright notice in the Description page of Project Settings.

#include "Arena/Damageable.h"
#include "Arena/ArenaUtilities.h"
#include "Arena/ArenaManager.h"
#include "Arena/TurnsManager.h"
#include "Arena/BattleLog/BattleLog.h"
#include "UnknownCommon.h"


UDamageable::UDamageable(const FObjectInitializer& objectInitializer)
    : Super(objectInitializer)
    , mMaxHP(100)
    , mCurrentHP(mMaxHP)
    , mIsImmortal(false)
{
    PrimaryComponentTick.bCanEverTick = false;
}

void UDamageable::DuplicateState(const UDamageable* source)
{
    mMaxHP = source->mMaxHP;
    mCurrentHP = source->mCurrentHP;
    mDefenceClear = source->mDefenceClear;
    mDefenceBonus = source->mDefenceBonus;
    mResistClear = source->mResistClear;
    mResistBonus = source->mResistBonus;
    mIsImmortal = source->mIsImmortal;
}

void UDamageable::PushDamageableTask(const FDamageableTask& damageableTask)
{
    mDamageableTasks.Push(damageableTask);
    mShouldExecuteTasks = true;
}

void UDamageable::ExecuteDamageableTask(const FDamageableTask& damageableTask)
{
    switch (damageableTask.mTaskType)
    {
    case EDamageableTaskType::Damage:
        {
            FDamageParameters deliveredDamage;
            Damage(damageableTask.mValue, damageableTask.mpInitiator, deliveredDamage);
            if (IsValid(damageableTask.mpResultBattleLogItem))
            {
                UBattleLogItem_Base* pBattleLogItem = damageableTask.mpResultBattleLogItem;
                pBattleLogItem->SetDeliveredDamage(deliveredDamage);
            }
        }
        break;

    case EDamageableTaskType::AddResistBonus:
        AddResistBonus(damageableTask.mValue);
        break;

    case EDamageableTaskType::AddDefenceBonus:
        AddDefenceBonus(damageableTask.mValue);
        break;

    case EDamageableTaskType::Heal:
        Heal(damageableTask.mValue.mAbsolute, damageableTask.mpInitiator);
        break;
    }
}

int32 UDamageable::Damage(const FDamageParameters& damage,
                          AActor* pDealer,
                          FDamageParameters& outDeliveredDamage)
{
    if (IsImmortal())
    {
        return 0;
    }

    outDeliveredDamage = UDamageOperator::CalculateDamage(damage,
                                                          GetResistFull(),
                                                          GetDefenceFull());

    int32 hpChange = -outDeliveredDamage.ComponentsSum();
    int32 beforeDamageHP = mCurrentHP;
    SetHP(mCurrentHP + hpChange);
    return beforeDamageHP - mCurrentHP;
}

void UDamageable::Heal(int32 hp,
                       AActor* pDealer)
{
    //	Dead can't be healed:
    if (IsAlive() == false)
    {
        return;
    }
    hp = FMath::Abs(hp);
    SetHP(mCurrentHP + hp);
    mEventHPChanged.Broadcast(this, hp, pDealer);
}

void UDamageable::BeginPlay()
{
    Super::BeginPlay();

    mMaxHP = mMaxHP;
    mCurrentHP = mMaxHP;
    mNeedDispatchDeathEvent = mCurrentHP > 0;

    UNKNOWN_NOT_ARENA_EDITOR_BEGIN(this)

        if (UArenaUtilities::AreSystemsSetup(this))
        {
            OnSystemsSetup();
        }
        else
        {
            UArenaUtilities::GetArenaManager(this)->mEventSystemsSetup.AddDynamic(this, &UDamageable::OnSystemsSetup);
        }

    UNKNOWN_NOT_ARENA_EDITOR_END
}

void UDamageable::SetHP(int32 newHP)
{
    mCurrentHP = FMath::Clamp(newHP, 0, mMaxHP);
}

void UDamageable::OnSystemsSetup()
{
    //  ~!~ If tile occupation release on death is implemented bad in PassMap - this may cause bugs as it is called "at the same time" as PassMap updates tile occupations.
    ATurnsManager* pTurnsManager = UArenaUtilities::GetTurnsManager(this);
    pTurnsManager->mEventUpdateDamageable.AddUObject(this, &UDamageable::OnUpdateDamageable);
}

void UDamageable::OnUpdateDamageable()
{
    //  ~TODO~ Remove it as it is called while OnFinalizeTurn and causes cross-system interaction:
    if (IsImmortal() == false &&
        IsAlive() == false &&
        mNeedDispatchDeathEvent)
    {
        mNeedDispatchDeathEvent = false;
        mEventDeath.Broadcast(this);
    }
}

bool UDamageable::IsAlive() const
{
    //  ~UGLY~ If called before all tasks are executed:
    if (mShouldExecuteTasks)
    {
        //  ~!~BUG~ May cause bug as it can be called before all tasks will be submitted for the pending turn.
        //  ~TODO~ Remove it. It was added for TiledMovement component to be able to use it while PassMap update.
        const_cast<UDamageable*>(this)->ExecuteTasks();
    }
    return mCurrentHP > 0;
}

int32 UDamageable::GetNominalMaxHP() const
{
    return mMaxHP;
}

int32 UDamageable::GetCurrentMaxHP() const
{
    return mMaxHP;
}

int32 UDamageable::GetCurrentHP() const
{
    return mCurrentHP;
}

const FDamageParameters& UDamageable::GetResistClear() const
{
    return mResistClear;
}

void UDamageable::AddResistBonus(const FDamageParameters& addBonus)
{
    mResistBonus += mResistBonus;
}

void UDamageable::SetResistBonus(const FDamageParameters& newBonus)
{
    mResistBonus = newBonus;
}

const FDamageParameters& UDamageable::GetResistBonus() const
{
    return mResistBonus;
}

FDamageParameters UDamageable::GetResistFull() const
{
    return mResistClear + mResistBonus;
}

const FDamageParameters& UDamageable::GetDefenceClear() const
{
    return mDefenceClear;
}

void UDamageable::AddDefenceBonus(const FDamageParameters& addBonus)
{
    mDefenceBonus += addBonus;
}

void UDamageable::SetDefenceBonus(const FDamageParameters& newBonus)
{
    mDefenceBonus = newBonus;
}

const FDamageParameters& UDamageable::GetDefenceBonus() const
{
    return mDefenceBonus;
}

FDamageParameters UDamageable::GetDefenceFull() const
{
    return mDefenceClear + mDefenceBonus;
}

void UDamageable::SetImmortal(bool isImmortal)
{
    mIsImmortal = isImmortal;
}

bool UDamageable::IsImmortal() const
{
    return mIsImmortal;
}

void UDamageable::Revive(int32 hpAfterRevive)
{
    //  Alive or immortal can't be revived:
    if (IsImmortal() == false &&
        IsAlive() &&
        hpAfterRevive > 0)
    {
        SetHP(mCurrentHP);
        mNeedDispatchDeathEvent = true;
        mEventRevive.Broadcast(this);
    }
}

void UDamageable::StoreState_Inner(ArenaStateContext& arenaState, State_Damageable& state) const
{
    state.mCurrentHP = mCurrentHP;
    state.mMaxHP = mMaxHP;
    state.mDefenceClear = mDefenceClear;
    state.mDefenceBonus = mDefenceBonus;
    state.mResistClear = mResistClear;
    state.mResistBonus = mResistBonus;
    state.mIsImmortal = mIsImmortal;
}

bool UDamageable::RestoreState_Inner(const State_Damageable& state)
{
    mMaxHP = state.mMaxHP;
    SetHP(state.mCurrentHP);
    mDefenceClear = state.mDefenceClear;
    mResistClear = state.mResistClear;
    SetDefenceBonus(state.mDefenceBonus);
    SetResistBonus(state.mResistBonus);
    SetImmortal(state.mIsImmortal);
    return true;
}

void UDamageable::ExecuteTasks()
{
    mShouldExecuteTasks = false;
    mDamageableTasks.Sort([](const FDamageableTask& left, const FDamageableTask& right) {
        return left.mPriority < right.mPriority;
    });
    for (const FDamageableTask& task : mDamageableTasks)
    {
        ExecuteDamageableTask(task);
    }
    mDamageableTasks.Empty();
}
