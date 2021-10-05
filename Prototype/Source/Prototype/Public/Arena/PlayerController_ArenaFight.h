// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "PlayerController_ArenaFight.generated.h"


class AAction_Base;
class ACreatureBase;
class APassMap;
class APlayerState_Arena;
class AParty;


UCLASS(Blueprintable, Category = "Arena|Party")
class PROTOTYPE_API APlayerController_ArenaFight
    : public APlayerController
{
    GENERATED_BODY()
    
public:

    DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FEventPartyChanged, AParty*, pOldParty, AParty*, pNewParty);
    UPROPERTY(BlueprintAssignable, Category = "Arena")
    FEventPartyChanged mEventPartyChanged;

    DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FEventFocusedMemberChanged, ACreatureBase*, pOldFocused, ACreatureBase*, pNewFocused);
    UPROPERTY(BlueprintAssignable, Category = "Arena")
    FEventFocusedMemberChanged mEventFocusedMemberChanged;

    DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FEventFocusedActionChanged, AAction_Base*, oldFocusedAction, AAction_Base*, newFocusedAction);
    UPROPERTY(BlueprintAssignable, Category = "Arena")
    FEventFocusedActionChanged mEventFocusedActionChanged;

    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FEventMembersUpdated, AParty*, pParty);
    UPROPERTY(BlueprintAssignable, Category = "Arena")
    FEventMembersUpdated mEventMembersUpdated;

    DECLARE_DYNAMIC_MULTICAST_DELEGATE(FEventPlayerControlChanged);
    UPROPERTY(BlueprintAssignable, Category = "Arena")
    FEventPlayerControlChanged mEventPlayerControlLocked;

    UPROPERTY(BlueprintAssignable, Category = "Arena")
    FEventPlayerControlChanged mEventPlayerControlUnlocked;

    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FEventActionToExecuteSetup, AAction_Base*, actionToExecute);
    UPROPERTY(BlueprintAssignable, Category = "Arena")
    FEventActionToExecuteSetup mEventActionToExecuteSetup;

public:

    APlayerController_ArenaFight(const FObjectInitializer& objectInitializer = FObjectInitializer::Get());

    void Tick(float deltaSeconds) override;

    void InitPlayerState() override;

    UFUNCTION(BlueprintPure, Category = "Arena")
    APlayerState_Arena* GetArenaPlayerState() const;

    UFUNCTION(BlueprintPure, Category = "Arena")
    AParty* GetPossessedParty() const;

    UFUNCTION(Client, Reliable)
    void Client_NotifyOpponentQuit();

    UFUNCTION(BlueprintPure, Category = "Arena")
    bool GetIsPlayerControlLocked() const;

protected:

    UPROPERTY(BlueprintReadOnly, Category = "Arena")
    APlayerState_Arena* mpArenaPlayerState;

    UPROPERTY(BlueprintReadOnly, Category = "Arena")
    AParty* mpPossessedParty;

    UPROPERTY(BlueprintReadOnly, Category = "Arena")
    ACreatureBase* mpFocusedMember;

    UPROPERTY(BlueprintReadOnly, Category = "Arena")
    bool mIsPlayerControlLocked = true;

protected:

    void BeginPlay() override;

    void OnPossess(APawn* pPossessedPawn) override;

    void OnUnPossess() override;

    void OnRep_Pawn() override;

    void OnRep_PlayerState() override;

    void SetupInputComponent() override;

    UFUNCTION(BlueprintNativeEvent, Category = "Arena")
    void OnArenaSetup();
    virtual void OnArenaSetup_Implementation();

    UFUNCTION(BlueprintCallable, Category = "Arena")
    void SetIsPlanningFinished(bool isFinished);

    UFUNCTION(BlueprintImplementableEvent, Category = "Arena", meta=(DisplayName="LockPlayerControl"))
    void K2_LockPlayerControl();

    UFUNCTION()
    void UnlockPlayerControl();

    UFUNCTION(BlueprintImplementableEvent, Category = "Arena", meta=(DisplayName="UnlockPlayerControl"))
    void K2_UnlockPlayerControl();

    UFUNCTION()
    void OnPreExecution();

    UFUNCTION()
    void OnTurnFinished();

    UFUNCTION(BlueprintImplementableEvent)
    void OnEndOfArena();

    UFUNCTION(BlueprintImplementableEvent)
    void OnPartyChanged(AParty* pOldParty);

    UFUNCTION(BlueprintCallable, Category = "Arena")
    void SwitchHotSeatParty();

    UFUNCTION(Server, Reliable, WithValidation)
    void Server_SwitchHotSeatParty();

    UFUNCTION(BlueprintCallable, Category = "Arena")
    void SwitchFocusedMember(bool forwardIteration);

    UFUNCTION(BlueprintCallable, Category = "Arena")
    void SetFocusedMember(ACreatureBase* pExistingMember);

    UFUNCTION(BlueprintCallable, Category = "Arena")
    void SetFocusedMemberByIndex(int32 memberIndex);

    UFUNCTION(BlueprintPure, Category = "Arena")
    void GetFocusedMember(ACreatureBase*& pFocusedMember, int32& memberIndex);

    UFUNCTION(BlueprintCallable, Category = "Arena")
    void ResetFocusedMember(bool unfocusAll = false);

    UFUNCTION()
    void OnMembersUpdated(AParty* pParty);

    //

    UFUNCTION(Server, Reliable, WithValidation)
    void Server_NotifyArenaSetup();

    UFUNCTION(Server, Reliable, WithValidation, Category = "Arena")
    void Server_SetIsPlanningFinished(bool isPlanningFinished);

    UFUNCTION(Server, Reliable, WithValidation)
    void Server_NotifyFinishedExecution();

    //  EoA stands for End of Arena. Call to ask server to retry arena fight. If both players choose retry - server travels back to the ArenaLobby.
    UFUNCTION(Server, Reliable, WithValidation, BlueprintCallable, Category = "Arena")
    void Server_EoARetry();

    UFUNCTION(Client, Reliable)
    void Client_UnlockPlayerControl();

    UFUNCTION(BlueprintImplementableEvent, Category = "Arena", meta=(DisplayName="NotifyOpponentQuit"))
    void K2_NotifyOpponentQuit();

private:

    void SetPossessedParty(AParty* pNewParty);

    bool FindNextFocusableMember(int32 startIndex, bool forwardIteration, int32& focusableMemberIndex);

    void ValidateFocusedMember();

    void LockPlayerControl();

    //
    //  Action setup:
    //

protected:

    UPROPERTY(BlueprintReadWrite, Meta = (DisplayName = "FocusedAction"))
    AAction_Base* mFocusedAction = nullptr;

protected:

    UFUNCTION(BlueprintCallable)
    void SetFocusedAction(AAction_Base* newFocusedAction);

    UFUNCTION(BlueprintImplementableEvent, Meta = (DisplayName = "OnFocusedActionChanged"))
    void K2_OnFocusedActionChanged(AAction_Base* oldFocusedAction,
                                   AAction_Base* newFocusedAction);

    UFUNCTION(BlueprintCallable)
    AAction_Base* GetFocusedAction() const;

    void StartActionSetup();

    //  Returns true if movement setup is finished.
    void PushMovementNextStep(const FIntPoint& stepTile);

    //  Returns true if movement setup is finished.
    void SetMovementDestination(const FIntPoint& destTile);

    void ResetActionSetup();

    void FinishActionSetup();

    void OnActionSetupPickMovementStepTile();

    void OnActionSetupPickMovementDestinationTile();

    void OnActionSetupPickExecutionDirection();

    void OnShowAllPartyMembersActionsSetup();

    void OnHideAllPartyMembersActionsSetup();

private:

    UPROPERTY()
    APassMap* mPassMap = nullptr;

    TArray<FIntPoint> mMovementPath;

    int32 mExecutionDirection = 0;

    FIntPoint mTileUnderCursor;

    TArray<uint8> mMovementDistanceField;

private:

    void UpdateTileUnderCursor();

    //void UpdateMovementDistanceFieldForFocusedMember();

    FIntPoint mRayMarchFirstTile;
    FIntPoint mRayMarchLastTile;
    bool mFlipFlopRayMarch = true;
};
