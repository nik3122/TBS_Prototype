// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "TurnsManager.generated.h"


class AActionMap;


UCLASS(Blueprintable)
class PROTOTYPE_API ATurnsManager:
    public AInfo
{
    GENERATED_BODY()

public:

    ATurnsManager(const FObjectInitializer& objInit);

    DECLARE_MULTICAST_DELEGATE(FEventApllyStatusEffects);
    FEventApllyStatusEffects mEventApplyStatusEffectsBeforeActions;

    DECLARE_DELEGATE(FEventApplyActionsPreExecution);
    FEventApplyActionsPreExecution mEventApplyActionsPreExecution;

    //  On this event all objects which need to have visual execution during the "Turn Execution Phase" should call TurnsManager::QueryTurnExecution().
    //  In addition PreExecutionApply() is called for all submitted actions.
    //  This is the last event of "Turn Planning Phase".
    DECLARE_DYNAMIC_MULTICAST_DELEGATE(FEventPreExecution);
    UPROPERTY(BlueprintAssignable, Category = "Arena")
    FEventPreExecution mEventPreExecution;

    //  If SkipExecution == true this event is NOT triggered. This event starts "Turn Execution Phase". While this phase all objects play animations, spawns VFX and do other visual stuff.
    DECLARE_DYNAMIC_MULTICAST_DELEGATE(FEventStartExecution);
    UPROPERTY(BlueprintAssignable, Category = "Arena")
    FEventStartExecution mEventStartExecution;

    //  If SkipExecution == true this event is NOT triggered. This event stops "Turn Execution Phase". All animations, VFXs and other visual stuff should be stopped/removed.
    //  If SkipExecution == false this event is triggered when all turn executor objects call ATurnsManager::ReleaseTurnExecution().
    DECLARE_DYNAMIC_MULTICAST_DELEGATE(FEventStopExecution);
    UPROPERTY(BlueprintAssignable, Category = "Arena")
    FEventStopExecution mEventStopExecution;

    //  On this event all factual changes of the game state can be applied by any objects.
    //  In addition PostExecutionApply() is called for all submitted actions.
    DECLARE_DYNAMIC_MULTICAST_DELEGATE(FEventPostExecution);
    UPROPERTY(BlueprintAssignable, Category = "Arena")
    FEventPostExecution mEventPostExecution;

    FEventApllyStatusEffects mEventApplyStatusEffectsAfterActions;

    //  Event for updating damageable.
    DECLARE_MULTICAST_DELEGATE(FEventUpdateDamageable);
    FEventUpdateDamageable mEventUpdateDamageable;

    //  This event can be used by systems to update their states with results of the executed turn.
    //  E.g. DamageableComponents set final owner's HP, StatusEffectsManagers executes activate/deactivate tasks, PassMap refreshes occupations.
    DECLARE_DYNAMIC_MULTICAST_DELEGATE(FEventFinalizeTurn);
    UPROPERTY(BlueprintAssignable, Category = "Arena")
    FEventFinalizeTurn mEventFinalizeTurn;

    //  This event is called after the turn is finalized to notify either GameMode (if listen or dedicated server or standalone)
    //  or PlayerController (if client) that the turn has been completelly finished. Clients notifies server about that then.
    DECLARE_DELEGATE(FEventTurnFinished)
    FEventTurnFinished mEventTurnFinished;

    //  On this event some preparations for the "Turn Planning Phase" can be made. E.g. Projectile_Base::mIsMissing is set to random value once a turn, to be genuine "random".
    DECLARE_DYNAMIC_MULTICAST_DELEGATE(FEventPrepareForPlanning);
    UPROPERTY(BlueprintAssignable, Category = "Arena")
    FEventPrepareForPlanning mEventPrepareForPlanning;

    DECLARE_DELEGATE(FEventUnlockPlayerControl)
    FEventUnlockPlayerControl mEventUnlockPlayerControl;

    DECLARE_DELEGATE(FEventStoreTurnState);
    FEventStoreTurnState mEventStoreTurnStateInitial;
    FEventStoreTurnState mEventStoreTurnStateResolved;
    FEventStoreTurnState mEventStoreTurnStateFinal;

public:

    void SetupSystem(AActionMap* pActionMap);

    //  ~CLIENT~
    //  Should be called by any object which is executing turn (visual FX, actions and so on.).
    //  Can be called after turn execution started.
    UFUNCTION(BlueprintCallable, Category = "Arena")
    void QueryTurnExecution(UPARAM(DisplayName = "Turn Maker Object (optional)") UObject* pTurnMaker);

    //  ~CLIENT~
    //  Each object called QueryTurnExecution should call ReleaseTurnExecution when it finished execution.
    //  When internal counter of executing objects 
    UFUNCTION(BlueprintCallable, Category = "Arena")
    void ReleaseTurnExecution(UPARAM(DisplayName = "Turn Maker Object (optional)") UObject* pTurnMaker);

    void StartPlanning();

    //  ~?~T~ Better to remove in the future and let the GameMode_ArenaFight to lock/unlock
    //  player controller dirrectly.
    void FinishPlanning();

    void ExecuteTurn();

    UFUNCTION(BlueprintCallable, Category = "Arena")
    void InterruptExecution();

    UFUNCTION(BlueprintPure, Category = "Arena")
    int32 GetCurrentTurn() const;

    //  0 means no turns limit.
    UFUNCTION(BlueprintPure, Category = "Arena")
    int32 GetTurnsLimit() const;

    UFUNCTION(BlueprintPure, Category = "Arena")
    float GetLastTurnDuration() const;

    UFUNCTION(BlueprintPure, Category = "Arena")
    bool IsExecuting() const;

    void SetSkipExecution(bool skipExecution);

protected:

    UPROPERTY()
    int32 mCurrentTurn = 0;

    //  0 means no turns limit.
    UPROPERTY()
    int32 mTurnsLimit = 0;

    UPROPERTY()
    float mExecutionDuration = 0.0f;

    UPROPERTY()
    TSet<UObject*> mTurnExecutors;

    float mAutoTurnEndOvertime = 15.0f;

    bool mIsExecuting = false;

    bool mSkipExecution = false;

protected:

    void BeginPlay() override;

    void Tick(float deltaTime) override;

    void TryStopTurnExecution(bool forceEndTurn = false);

    void StopTurnExecution();

    UFUNCTION(NetMulticast, Reliable)
    void Multicast_StartPlanning();

    UFUNCTION(NetMulticast, Reliable)
    void Multicast_FinishPlanning();

    FString GetTurnExecutorName(UObject* pTurnExecutor) const;
};
