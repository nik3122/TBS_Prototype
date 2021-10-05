// Fill out your copyright notice in the Description page of Project Settings.

#include "Arena/ActionManager.h"
#include "Arena/Action_Idle.h"
#include "Arena/ActionMap.h"
#include "Arena/ActionManager.h"
#include "Arena/ArenaManager.h"
#include "Arena/ArenaUnit_Interface.h"
#include "Arena/ArenaUnitAttributes.h"
#include "Arena/ArenaUtilities.h"
#include "Arena/Damageable.h"
#include "Arena/PassMap.h"
#include "Arena/TiledMovement.h"
#include "Arena/TurnsManager.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "UnknownCommon.h"


uint32 UActionManager::sUniqueActionID = 1;

UActionManager::UActionManager(const FObjectInitializer& objectInitializer)
    : Super(objectInitializer)
    , mpActionToExecute(nullptr)
    , mNominalReactionRate(0)
    , mReactionRateBonus(0)
    , mpIdleAction(nullptr)
    , mExecutionPriority(0)
{
    //bAutoActivate = true;
}

void UActionManager::SyncAndSubmitActionToExecute()
{
    Client_RequestActionToExecute();
}

void UActionManager::BeginPlay()
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
        UArenaUtilities::GetArenaManager(this)->mEventSystemsSetup.AddDynamic(this, &UActionManager::OnSystemsSetup);
    }

    UNKNOWN_NOT_ARENA_EDITOR_END
}

void UActionManager::EndPlay(const EEndPlayReason::Type endPlayReason)
{
    UNKNOWN_NOT_ARENA_EDITOR_BEGIN(this)

    if (endPlayReason == EEndPlayReason::Destroyed &&
        UArenaUtilities::AreSystemsSetup(this))
    {
        UArenaUtilities::GetActionMap(this)->UnregisterActionManager(this);
    }

    UNKNOWN_NOT_ARENA_EDITOR_END

    Super::EndPlay(endPlayReason);
}

void UActionManager::DestroyComponent(bool promoteChildren)
{
    Super::DestroyComponent(promoteChildren);

    ResetActionToExecute();
    for (auto* pAction : mSpawnedActions)
    {
        pAction->Destroy();
    }
}

void UActionManager::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(UActionManager, mSpawnedActions);
}

void UActionManager::OnSystemsSetup()
{
    ATurnsManager* pTurnsManager = UArenaUtilities::GetTurnsManager(this);
    pTurnsManager->mEventPrepareForPlanning.AddUniqueDynamic(this, &UActionManager::OnPrepareForPlanning);
    pTurnsManager->mEventPostExecution.AddUniqueDynamic(this, &UActionManager::OnPostExecution);
    UArenaUtilities::GetActionMap(this)->RegisterActionManager(this);
    ResetActionToExecute();

    //  Spawn actions:

    mpIdleAction = Cast<AAction_Idle>(SpawnAction(AAction_Idle::StaticClass()));

    if (GetOwner()->HasAuthority())
    {
        for (auto& actionType : mNativeActionTypes)
        {
            AAction_Base* action = SpawnAction(actionType);
            mSpawnedActions.Push(action);
        }

        for (auto& actionType : mActionTypes)
        {
            AAction_Base* action = SpawnAction(actionType);
            mSpawnedActions.Push(action);
        }
    }
}

const TArray<AAction_Base*>& UActionManager::GetAllActions()
{
    return mSpawnedActions;
}

int32 UActionManager::GetAllReadyActions(TArray<AAction_Base*>& outActions)
{
    int32 actionsCount = 0;
    for (auto* pAction : mSpawnedActions)
    {
        if (pAction->IsReloaded())
        {
            outActions.Push(pAction);
            ++actionsCount;
        }
    }
    return actionsCount;
}

void UActionManager::SetActionToExecute(AAction_Base* pAction)
{
    check(pAction != nullptr && "ERROR: UTurnBasedController::SetPrimaryAction() pAction == nullptr.");
    check(pAction->IsSetup() && "UActionManager: Try to set action which is not setup.");
    //	~!~ If a new action equals already setup one - no reset is needed:
    if (pAction != mpActionToExecute)
    {
        ResetActionToExecute();
        mpActionToExecute = pAction;
        mEventSetActionToExecute.Broadcast(this, mpActionToExecute);
    }
}

AAction_Base* UActionManager::GetActionToExecute() const
{
    return mpActionToExecute;
}

void UActionManager::ResetActionToExecute()
{
    if (mpActionToExecute != nullptr)
    {
        mpActionToExecute->Reset();
        mpActionToExecute = nullptr;
        mEventSetActionToExecute.Broadcast(this, mpActionToExecute);
    }
}

int32 UActionManager::GetNominalReactionRate() const
{
    return mNominalReactionRate;
}

void UActionManager::AddReactionRateBonus(int32 reactionRateBonus)
{
    mReactionRateBonus += reactionRateBonus;
}

void UActionManager::SetReactionRateBonus(int32 reactionRateBonus)
{
    mReactionRateBonus = reactionRateBonus;
}

int32 UActionManager::GetReactionRateBonus() const
{
    return mReactionRateBonus;
}

int32 UActionManager::GetCurrentReactionRate() const
{
    return mNominalReactionRate + mReactionRateBonus;
}

int32 UActionManager::GetExecutionPriority() const
{
    check(mExecutionPriority > 0 && "It is valid only during an execution phase.");
    /*if (mExecutionPriority == 0)
    {
        UE_LOG(ArenaFightCommonLog, Error, TEXT("Probably UActionManager::GetExecutionPriority() is called not at execution phase. IT SHOULD BE CALLED ONLY WHILE EXECUTION PHASE"));
    }*/
    return mExecutionPriority;
}

void UActionManager::OnPostExecution()
{
    ProcessActionManagementTasks();
    UpdateAllActionsReloadTimer();
}

void UActionManager::OnPrepareForPlanning()
{
    UArenaUnitAttributes* arenaUnitAttributes = IArenaUnit_Interface::Execute_GetArenaUnitAttributes(GetOwner());
    int32 additionalInitiative = arenaUnitAttributes != nullptr ? arenaUnitAttributes->GetAgility() : 0;

    for (auto* pAction : mSpawnedActions)
    {
        if (IsValid(pAction))
        {
            pAction->SetAdditionalInitiative(additionalInitiative);
            pAction->PrepareForPlanning();
        }
        else
        {
            UE_LOG(ArenaCommonLog, Error, TEXT("%s @ UActionManager::OnPrepareForPlanning() spawned action is invalid."), *GetOwner()->GetName());
        }
    }

    ResetActionToExecute();
    mExecutionPriority = 0;
}

void UActionManager::Client_RequestActionToExecute_Implementation()
{
    UTiledMovement* pExecutorPassMapMovement = IArenaUnit_Interface::Execute_GetTiledMovement(GetOwner());
    check(pExecutorPassMapMovement != nullptr && "Each action executor should have TiledMovement component.");
    int32 actionToExecuteIndex = mSpawnedActions.Find(mpActionToExecute);
    if (actionToExecuteIndex == INDEX_NONE)
    {
        Server_SetActionToExecute(-1, {});
    }
    else
    {
        Server_SetActionToExecute(actionToExecuteIndex,
                                  mpActionToExecute->GetActionSetupData());
    }
}

bool UActionManager::Server_SetActionToExecute_Validate(int32 actionIndex,
                                                        const FActionSetupData& actionSetupData)
{
    if (UArenaUtilities::GetActionMap(this)->CanSubmitAction() &&
        (mSpawnedActions.IsValidIndex(actionIndex) || actionIndex == -1))
    {
        APassMap* pPassMap = UArenaUtilities::GetPassMap(this);
        const TArray<FIntPoint>& actionSetupTiles = actionSetupData.mMovementTiles;
        for (const FIntPoint& tile : actionSetupTiles)
        {
            if (pPassMap->IsTileValid(tile) == false) return false;
        }
        return true;
    }
    return false;
}

void UActionManager::Server_SetActionToExecute_Implementation(int32 actionIndex,
                                                              const FActionSetupData& actionSetupData)
{
    mActionToExecuteIndex = actionIndex;
    //  Now distribute action to execute info across all clients and the server:
    Multicast_SetupAndSubmitActionToExecute(actionIndex, actionSetupData);
}

void UActionManager::Multicast_SetupAndSubmitActionToExecute_Implementation(int32 actionIndex,
                                                                            const FActionSetupData& actionSetupData)
{
    const TArray<FIntPoint>& actionSetupTiles = actionSetupData.mMovementTiles;

    bool isSetupSucceeded = false;
    if (actionIndex == -1)
    {
        mpActionToExecute = mpIdleAction;
        UTiledMovement* pExecutorPassMapMovement = IArenaUnit_Interface::Execute_GetTiledMovement(GetOwner());
        check(pExecutorPassMapMovement != nullptr && "Each action executor should have TiledMovement component.");
        mpIdleAction->SetPlacement(pExecutorPassMapMovement->GetCurrentTile());
        isSetupSucceeded = mpActionToExecute->Setup();
    }
    else
    {
        mpActionToExecute = mSpawnedActions[actionIndex];
        mpActionToExecute->SetActionSetupData(actionSetupData);
        isSetupSucceeded = mpActionToExecute->Setup();
    }
    check(isSetupSucceeded && "Failed to setup action after replication.");
    UArenaUtilities::GetActionMap(this)->SubmitAction(this, mpActionToExecute, mExecutionPriority);
}

void UActionManager::AddAction(const TSubclassOf<AAction_Base>& actionType,
                               int32 availabilityDurationInTurns)
{
    check(*actionType != nullptr && "Trying to add invalid action type.");
    check(availabilityDurationInTurns > 0);
    ActionManagementTask task;
    task.mActionType = actionType;
    task.mAvailabilityDuration = availabilityDurationInTurns;
    task.mTimeOut = 0;
    mActionManagementTasksQueue.Push(task);
}

void UActionManager::RemoveAction(const TSubclassOf<AAction_Base>& actionType)
{
    check(*actionType != nullptr && "Trying to add invalid action type.");
    ActionManagementTask task;
    task.mActionType = actionType;
    task.mAvailabilityDuration = 0;
    task.mTimeOut = 0;
    mActionManagementTasksQueue.Push(task);
}

void UActionManager::StoreState_Inner(ArenaStateContext& context, State_ActionManager& state) const
{
    state.mActionsCount = mSpawnedActions.Num();
    state.mppActionsStates = state.mActionsCount > 0 ? context.AllocMemory<State_Action_Base*>(state.mActionsCount) : nullptr;
    for (int32 i = 0; i < state.mActionsCount; ++i)
    {
        state.mppActionsStates[i] = mSpawnedActions[i]->StoreState(context);
    }
    state.mNominalReactionRate = mNominalReactionRate;
    state.mReactionRateBonus = mReactionRateBonus;
    state.mActionToExecuteIndex = -1;
    for (int32 i = 0; i < state.mActionsCount; ++i)
    {
        if (mSpawnedActions[i] == mpActionToExecute)
        {
            state.mActionToExecuteIndex = i;
            break;
        }
    }
}

bool UActionManager::RestoreState_Inner(const State_ActionManager& state)
{
    return true;
}

uint32 UActionManager::GetNextActionID() const
{
    return GenerateActionID();
}

uint32 UActionManager::GenerateActionID()
{
    return sUniqueActionID++;
}

AAction_Base* UActionManager::SpawnAction(const TSubclassOf<AAction_Base>& actionClass)
{
    uint32 actionID = GetNextActionID();
    FActorSpawnParameters spawnInfo;
    spawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    spawnInfo.Name = *(actionClass->GetName() + TEXT("_") + FString::FromInt(actionID));
    spawnInfo.Owner = GetOwner();
    spawnInfo.Instigator = nullptr;
    spawnInfo.bDeferConstruction = true;
    AAction_Base* spawnedAction = GetWorld()->SpawnActor<AAction_Base>(actionClass.Get(),
                                                                       FTransform::Identity,
                                                                       spawnInfo);
    spawnedAction->SetExecutorActor(GetOwner());
    spawnedAction->SetActionID(actionID);
    UGameplayStatics::FinishSpawningActor(spawnedAction, FTransform::Identity);
    return spawnedAction;
}

void UActionManager::ProcessActionManagementTasks()
{
    TArray<ActionManagementTask> removeLaterTasks;
    //  ~?~HACK~ RemoveAll used as for-each loop:
    mActionManagementTasksQueue.RemoveAll([this, &removeLaterTasks](ActionManagementTask& task){
        check(*task.mActionType != nullptr);
        --task.mTimeOut;
        if (task.mTimeOut <= 0)
        {
            //  Remove:
            if (task.mAvailabilityDuration == 0)
            {
                RemoveAction_Inner(task.mActionType);
            }
            //  Add:
            else
            {
                bool alreadyExists = false;
                AddAction_Inner(task.mActionType, alreadyExists);
                if (alreadyExists == false &&
                    task.mAvailabilityDuration > 0)
                {
                    //  If action doesn't already exist - action is dynamically added
                    //  thus should be removed later:
                    ActionManagementTask removeTask;
                    removeTask.mActionType = task.mActionType;
                    removeTask.mAvailabilityDuration = 0;
                    removeTask.mTimeOut = task.mAvailabilityDuration;
                    removeLaterTasks.Push(removeTask);
                }
            }
        }
        return task.mTimeOut <= 0;
    });
    mActionManagementTasksQueue.Append(removeLaterTasks);
}

void UActionManager::AddAction_Inner(const TSubclassOf<AAction_Base>& actionType,
                                     bool& alreadyExists)
{
    alreadyExists = mSpawnedActions.ContainsByPredicate([actionType](const AAction_Base* pAction){
        return pAction->GetClass() == *actionType;
    });
    if (alreadyExists == false)
    {
        AAction_Base* pSpawnedAction = GetWorld()->SpawnActor<AAction_Base>(actionType);
        pSpawnedAction->SetExecutorActor(GetOwner());
        mSpawnedActions.Push(pSpawnedAction);
        mEventActionsSetChanged.Broadcast(this);
    }
}

void UActionManager::RemoveAction_Inner(const TSubclassOf<AAction_Base>& actionType)
{
    mSpawnedActions.RemoveAll([actionType](const AAction_Base* pSpawnedAction){
        return pSpawnedAction->GetClass() == *actionType;
    });
    mEventActionsSetChanged.Broadcast(this);
}

void UActionManager::UpdateAllActionsReloadTimer()
{
    for (AAction_Base* pAction : mSpawnedActions)
    {
        pAction->UpdateReloadTimer();
    }
    mpIdleAction->UpdateReloadTimer();
}

void UActionManager::OnRep_SpawnedActions()
{
    UE_LOG(ArenaCommonLog, VeryVerbose, TEXT("OnRep_SpawnedActions() spawned actions num = %i"), mSpawnedActions.Num());
    for (AAction_Base* pAction : mSpawnedActions)
    {
        if (IsValid(pAction))
        {
            pAction->SetExecutorActor(GetOwner());
            UE_LOG(ArenaCommonLog, VeryVerbose, TEXT("%s @ ActionManager: bound new action %s"), *GetOwner()->GetName(), *pAction->GetName());
        }
    }
    mEventActionsSetChanged.Broadcast(this);
}
