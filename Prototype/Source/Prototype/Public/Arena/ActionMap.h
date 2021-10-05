// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Arena/Action_Base.h"
#include "GameFramework/Actor.h"
#include "ActionMap.generated.h"


#define UNKNOWN_PRIORITY_UNIT_VALUE 4000


class UBattleLogItem_Base;
class ATurnsManager;


/**
 *  AActionMap acts like a registry for actions which are executed each turn.
 *  After all actions are submitted, AActionMap processes them to resolve conflicts between them.
 *  When the conflicts are processed actions may be executed to visualize the result.
 * 
 *  ~?~TODO~ The core interface may be split off (register action managers, submit actions, resolve conflicts, etc.).
 */
UCLASS()
class PROTOTYPE_API AActionMap
    : public AInfo
{
    GENERATED_BODY()

public:

    DECLARE_DELEGATE(FEventActionsSubmittingFinished);
    FEventActionsSubmittingFinished mEventActionsSubmittingFinished;

public:

    AActionMap();

    virtual void SetupSystem(ATurnsManager* pTurnsManager);

    void RegisterActionManager(UActionManager* pActionManager);

    void UnregisterActionManager(UActionManager* pActionManager);

    void StartActionSubmitting();

    bool CanSubmitAction() const;

    //	~?~ It is better to pass the action priority at the same time to get more flexebility,
    //	when other systems can modify this value without changing AActionMap. // , uint32 weight);
    void SubmitAction(UActionManager* pActionManager,
                      AAction_Base* pAction,
                      int32& outExecutionPriority);

    void ResolveActionConflicts();

    UFUNCTION(BlueprintPure, Category = "Arena")
    const TArray<AAction_Base*>& GetSubmittedActions() const;

    UFUNCTION(BlueprintPure, Category = "Arena")
    UBattleLog* GetBattleLog() const;

protected:

    /**
     *  Contains all submitted actions. After all actions are submitted, 
     *  they are sorted in the descending order of the execution priority.
     *  This way they appear already sorted at the beggining of ResolveActionConflicts().
     */
    UPROPERTY()
    TArray<AAction_Base*> mSubmittedActions;

    UPROPERTY()
    UBattleLog* mBattleLog = nullptr;

protected:

    void BeginPlay() override;

    virtual int32 CalculateActionPriority(AAction_Base* action);

    //  Returns action's execution priority.
    virtual void SubmitAction_Inner(AAction_Base* action,
                                    int32 executionPriority);

    virtual void ResolveActionConflicts_Inner();

    UFUNCTION()
    virtual void OnPrepareForPlanning();

    UFUNCTION()
    virtual void OnApplyActionsPreExecution();

    UFUNCTION()
    virtual void OnStartExecution();

    UFUNCTION()
    virtual void OnStopExecution();

    UFUNCTION()
    virtual void OnPostExecution();

    UFUNCTION()
    virtual void OnActionExecuted(AAction_Base* pExecutedAction);

    void FinishActionSubmitting();

private:

    bool mIsActionSubmittingAllowed = false;

    bool mIsResolvingCollisions = false;

    UPROPERTY()
    TArray<UActionManager*> mRegisteredActionManagers;

    TSet<UActionManager*> mExpectedSubmittingActionManagers;

};
