#pragma once


#include "Action_Base.h"
#include "Action_Spawn_Base.generated.h"


//
//  AAction_Spawn_Base is base class for action which spawns any arena influancers (arena units, traps etc.).
//  The core idea is about spawning required actor only on the server side which is replicated to clients then.
//  The action itself doesn't stop execution (even if StopExecution is called) untill target will be spawned and
//  replicated on the client. This way it guarantees that ReleaseTurnExecution won't be called earlier than 
//  the target actor will take part in the arena.
//
//  Child action must call TrySpawnLocally() when it wants to visualize spawned actor during the execution.
//  Underhood it just checks if target actor was replicated and if yes - sets Hidden = false.
UCLASS(Abstract)
class AAction_Spawn_Base
    : public AAction_Base
{
    GENERATED_BODY()

public:

    AAction_Spawn_Base(const FObjectInitializer& objectInitializer = FObjectInitializer::Get());

    //  AAction_Base::

    void Reset_Implementation() override;

    FActionCollisionResolveResult ResolveCollision_Implementation(AAction_Base* pLowPriorityAction,
                                                                  const FIntPoint& collisionPlacement,
                                                                  int32 highPrioEntryBehavior,
                                                                  int32 lowPrioEntryBehavior) override;

    void PreExecutionApply_Implementation() override;

    void StopExecution() override;

    TArray<UBattleLogItem_Base*> GenerateBattleLogItemsForCurrentTurn_Implementation() const override;

protected:

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Unknown|Arena")
    TSubclassOf<AActor> mActorToSpawnClass;

    UPROPERTY(EditDefaultsOnly, Category = "Unknown|Arena", Meta = (Bitmask, BitmaskEnum = "EOccupationSlot"))
    uint32 mOccupationSlotsToBeFree = 0;

    UPROPERTY(ReplicatedUsing = OnRep_SpawnedActors, BlueprintReadOnly, Category = "Unknown|Arena")
    TArray<AActor*> mSpawnedActors;

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Unknown|Arena")
    FName mSingleActionAnimationSection;

    //  Used to automatically call TrySpawnLocally(). Set <= 0.0f if not it's not needed.
    //  ~TODO~ Remove when animation events reaction will be implemented.
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Unknown|Arena")
    float mSpawnVisualDelay = 0.0f;

    UPROPERTY(EditDefaultsOnly, Category = "Unknown|Arena")
    bool mUseCustomActorReadinessCheck = false;

    //  Recomended to leave true.
    UPROPERTY(EditDefaultsOnly, Category = "Unknown|Arena")
    bool mForceUnhideSpawnedActorsOnStopExecution = true;

    //  If true single action animation will be played even if mIsSpawnAllowed == false.
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Unknown|Arena")
    bool mPlayAnimationAnyway = false;

    //  Customizable variable which acts like !mIsCanceled but only for spawning actors.
    //  Can be used by child actions to cancel only spawning mechanics.
    UPROPERTY(BlueprintReadWrite, Category = "Unknown|Arena")
    bool mIsSpawnAllowed = true;

    //  Indicates whether StopExecution() was called before.
    bool mIsExecutionFinished = false;

    bool mIsHiddenOnRep = true;

protected:

    void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    //  We rely on a single call of this function for the whole array.
    UFUNCTION()
    virtual void OnRep_SpawnedActors();

    //  Helper function which simply unhides the replicated actors if they are valid on a client.
    UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "Unknown|Arena")
    void TrySpawnLocally();

    //  ~CHILD~
    //  Called during the execution (if StopExecution hasn't benn called yet) when all actors are replicated locally.
    //  ~!~ It is for visuals only! No gameplay logics here!
    //  ~!~ All gameplay logics must be implemented in the PostExecutionApply if needed.
    UFUNCTION(BlueprintImplementableEvent, BlueprintCosmetic, Category = "Unknown|Arena")
    void OnActorsSpawnedLocally();

    //  Checks whether all actors were spawned and ready. Action execution may be finished only when it returns true.
    bool AreSpawnedActorsReady() const;

    //  ~CHILD~
    //  Custom actor readiness check.
    //  pActor : always valid spawned locally actor reference.
    UFUNCTION(BlueprintNativeEvent, Category = "Unknown|Arena")
    bool CustomActorReadinessCheck(AActor* pActor) const;
    bool CustomActorReadinessCheck_Implementation(AActor* pActor) const;

    //  AAction_Base::

    bool CanBeSetupAt_Implementation(const FIntPoint& potentialSetupTile) const override;

    bool TryDistributeAt_Implementation(const FIntPoint& potentialDistributionTile, int32& outBehaviorAtDistributionTile) override;

    bool FinalizeSetupProcess_Implementation() override;

    void OnCancel_Implementation(AAction_Base* pCanceledBy) override;

    bool OnInterrupt_Implementation(const FIntPoint& interruptionTile,
                                    AAction_Base* pInterruptedBy) override;

    bool IsPretendingOnTile_Implementation(const FIntPoint& tile) const override;

    void OnStartExecution_Implementation() override;

    void OnStopExecution_Implementation() override;

};