// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Action_Base.h"
#include "ArenaState.h"
#include "Version.h"
#include "ActionManager.generated.h"


class AAction_Base;
class AAction_Idle;
struct State_Action_Base;


struct State_ActionManager
    : public State_Base
{
    DECLARE_CLASS_VERSION(1, 0)
    int32 mNominalReactionRate;
    int32 mReactionRateBonus;
    State_Action_Base** mppActionsStates;
    int32 mActionsCount;
    int32 mActionToExecuteIndex;
};


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PROTOTYPE_API UActionManager
    : public UActorComponent
    , public ArenaStateObject_Base<State_ActionManager>
{
    CHECK_CLASS_VERSION_EQUAL(State_ActionManager, 1, 0)

    GENERATED_BODY()

public:

    DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FEventSetActionToExecute, UActionManager*, pActionManager, AAction_Base*, pAction);
    UPROPERTY(BlueprintAssignable, Category = "Arena")
    FEventSetActionToExecute mEventSetActionToExecute;

    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FEventActionsSetChanged, UActionManager*, pActionManager);
    UPROPERTY(BlueprintAssignable, Category = "Arena")
    FEventActionsSetChanged mEventActionsSetChanged;

public:

    UActionManager(const FObjectInitializer& objectInitializer = FObjectInitializer::Get());

    //  ~SERVER~ Called only for authority.
    void SyncAndSubmitActionToExecute();

    UFUNCTION(BlueprintCallable, Category = "Arena")
    const TArray<AAction_Base*>& GetAllActions();

    UFUNCTION(BlueprintCallable, Category = "Arena")
    int32 GetAllReadyActions(UPARAM(ref) TArray<AAction_Base*>& outActions);

    //	GetAvailableActionForPlacement(...)	- This should use ActionSetupContext structure to make exclusively fast setup.

    UFUNCTION(BlueprintCallable, Category = "Arena")
    void SetActionToExecute(AAction_Base* pAction);

    UFUNCTION(BlueprintPure, Category = "Arena")
    AAction_Base* GetActionToExecute() const;

    //  ~TODO~ Can be replaced with SetActionToExecute(nullptr) (or call SetActionToExecute_Inner(nullptr) without check for nullptr).
    UFUNCTION(BlueprintCallable, Category = "Arena")
    void ResetActionToExecute();

    UFUNCTION(BlueprintPure, Category = "Arena")
    int32 GetNominalReactionRate() const;

    UFUNCTION(BlueprintCallable, Category = "Arena")
    void AddReactionRateBonus(int32 reactionRateBonus);

    UFUNCTION(BlueprintCallable, Category = "Arena")
    void SetReactionRateBonus(int32 reactionRateBonus);

    UFUNCTION(BlueprintPure, Category = "Arena")
    int32 GetReactionRateBonus() const;

    UFUNCTION(BlueprintPure, Category = "Arena")
    int32 GetCurrentReactionRate() const;

    UFUNCTION(BlueprintPure, Category = "Arena")
    int32 GetExecutionPriority() const;

    UFUNCTION(BlueprintCallable, Category = "Arena")
    void AddAction(const TSubclassOf<AAction_Base>& actionType,
                   int32 availabilityDurationInTurns);

    UFUNCTION(BlueprintCallable, Category = "Arena")
    void RemoveAction(const TSubclassOf<AAction_Base>& actionType);

protected:

    //  Native action types are not supposed to be displayed in arena lobby at creature description panel (like MoveTo, possibly Fist and so on).
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Arena")
    TArray<TSubclassOf<AAction_Base>> mNativeActionTypes;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Arena")
    TArray<TSubclassOf<AAction_Base>> mActionTypes;

    UPROPERTY(ReplicatedUsing = OnRep_SpawnedActions, BlueprintReadWrite, Category = "Arena")
    TArray<AAction_Base*> mSpawnedActions;

    UPROPERTY()
    AAction_Base* mpActionToExecute;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Arena")
    int32 mNominalReactionRate;

    UPROPERTY(BlueprintReadOnly, Category = "Arena")
    int32 mReactionRateBonus;

    UPROPERTY(BlueprintReadOnly, Category = "Arena")
    AAction_Idle* mpIdleAction;

    int32 mExecutionPriority;

    int32 mActionToExecuteIndex;

protected:

    void BeginPlay() override;

    void EndPlay(const EEndPlayReason::Type endPlayReason) override;

    void DestroyComponent(bool promoteChildren) override;

    void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    UFUNCTION()
    void OnSystemsSetup();

    UFUNCTION()
    void OnPostExecution();

    UFUNCTION()
    void OnPrepareForPlanning();

    //  Process of Sync & Submit actions:
    //  [server] SyncAndSubmitActionsToExecute
    //  [server] Client_RequestActionToExecute
    //  [client] Server_SetActionToExecute
    //  [server] Multicast_SetupAndSubmitActionToExecute

    UFUNCTION(Client, Reliable)
    void Client_RequestActionToExecute();

    UFUNCTION(Server, Reliable, WithValidation)
    void Server_SetActionToExecute(int32 actionIndex,
                                   const FActionSetupData& actionSetupData);

    UFUNCTION(NetMulticast, Reliable)
    void Multicast_SetupAndSubmitActionToExecute(int32 actionIndex,
                                                 const FActionSetupData& actionSetupData);

    UFUNCTION()
    void OnRep_SpawnedActions();

private:

    struct ActionManagementTask
    {
        TSubclassOf<AAction_Base> mActionType;
        //  Adding temporary action - mAvailabilityDuration > 0
        //  Removing action - mAvailabilityDuration == 0
        //  Adding permanent action - mAvailabilityDuration < 0
        int32 mAvailabilityDuration = 0;
        int32 mTimeOut = 0;
    };

    TArray<ActionManagementTask> mActionManagementTasksQueue;

    static uint32 sUniqueActionID;

private:

    void StoreState_Inner(ArenaStateContext& context, State_ActionManager& state) const override;

    bool RestoreState_Inner(const State_ActionManager& state) override;

    uint32 GetNextActionID() const;

    static uint32 GenerateActionID();

    AAction_Base* SpawnAction(const TSubclassOf<AAction_Base>& actionClass);

    void ProcessActionManagementTasks();

    void AddAction_Inner(const TSubclassOf<AAction_Base>& actionType,
                         bool& alreadyExists);

    void RemoveAction_Inner(const TSubclassOf<AAction_Base>& actionType);

    void UpdateAllActionsReloadTimer();
};