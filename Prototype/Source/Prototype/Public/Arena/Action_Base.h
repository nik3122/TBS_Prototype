// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ArenaUnit_Interface.h"
#include "ArenaState.h"
#include "Version.h"
#include "StatusEffect.h"
#include "Damage.h"
#include "ActionCollisionResolveContext.h"
#include "Action_Base.generated.h"


class UActionManager;
class UTiledMovement;
class AActionMap;
class UActionDistributionPattern_Base;
class UBattleLogItem_Base;


#define UNKNOWN_ACTION_NEGATIVE_DAMAGE_RATE             -5
#define UNKNOWN_ACTION_POSITIVE_DAMAGE_RATE             5
#define UNKNOWN_ACTION_NEGATIVE_STATUS_EFFECTS_RATE     -2
#define UNKNOWN_ACTION_POSITIVE_STATUS_EFFECTS_RATE     2


UENUM(BlueprintType, Meta = (Bitflags))
enum class EActionBehavior : uint8
{
    ActionBehavior_Unknown,
    ActionBehavior_Executor,
    ActionBehavior_Move,
    ActionBehavior_Damage,
    ActionBehavior_Heal,
    ActionBehavior_Buff,
    ActionBehavior_Debuff,
    ActionBehavior_Summon,
    ActionBehavior_Trap,
    ActionBehavior_Mental,
    ActionBehavior_Defence,
};


#define UNKNOWN_IDLE_ACTION_BEHAVIOR 0xffffffff


/**
    The result of the collision resolve.
*/
UENUM(BlueprintType)
enum class EActionResolveResult: uint8
{
    ActionResolveResult_None,
    ActionResolveResult_Interrupt,
    ActionResolveResult_Cancel,

    ActionResolveResult_ForceCancel,
    ActionResolveResult_IncreaseCancellationCounter,
};


/**
    
*/
USTRUCT(BlueprintType)
struct FActionCollisionResolveResult
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Action")
    bool mInterrupt = false;

    UPROPERTY(BlueprintReadWrite, Category = "Action")
    bool mIncludingCollisionTile = true;

    UPROPERTY(BlueprintReadWrite, Category = "Action")
    int32 mCancellationCounterChange = false;
};


/**
    Describes behavior of an action at the tile.
*/
USTRUCT(BlueprintType)
struct FActionDistributionTile
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Action")
    FIntPoint mTile;

    UPROPERTY(BlueprintReadWrite, Category = "Action")
    int32 mEntryBehavior;

public:

    bool operator==(const FIntPoint& tile) const { return mTile == tile; }

    bool operator==(const FActionDistributionTile& distrTile) const { return mTile == distrTile.mTile && mEntryBehavior == distrTile.mEntryBehavior; }
};


/**
    Information for displaying in the card of an action.
    
    Each field will be displayed according to a specific behavior flag of an action.
*/
USTRUCT(BlueprintType)
struct FActionCardInfo
{
    GENERATED_BODY()

public:

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (DisplayName = "Action Name"))
    FText mName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (DisplayName = "Initiative"))
    int32 mInitiativeBase = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (DisplayName = "Reload Timer (turns #)"))
    int32 mReloadTimer = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (Bitmask, BitmaskEnum = "EActionBehavior", DisplayName = "Behavior Flags"))
    int32 mBehavior = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (DisplayName = "Related Status Effects"))
    TArray<TSubclassOf<AStatusEffect>> mStatusEffects;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (DisplayName = "Action Card Description"))
    FText mCardDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (DisplayName = "Action Card Picture"))
    UTexture2D* mpPicture;
};


struct State_Action_Base
    : public State_Base
{
    DECLARE_CLASS_VERSION(1, 1)
    const ANSICHAR* mpClassName;
    int32 mClassNameSize;
    FIntPoint* mSetupTiles;
    int32 mSetupTilesCount;
    bool mIsSetup;
};


/**
 *  FActionSetupData structure contains all info specified by player to setup an action.
 */
USTRUCT(BlueprintType)
struct FActionSetupData
{
    GENERATED_BODY()

public:

    UPROPERTY(BlueprintReadWrite)
    TArray<FIntPoint> mMovementTiles;

    //  Neighbour tile direction in which an action should be applied.
    UPROPERTY(BlueprintReadWrite)
    int32 mExecutionDirection = 0;
};


//
//  Base class for actions executed on arena on turn basis.
//
//  To define custom action you need to implement next methods:
//
//      Preparation phase:
//      + PrepareForPlanning
//      + Reset
//      + CanBeSetupAt
//      + TryDistributeAt
//      + FinalizeSetupProcess
//
//      Collision resolving:
//      + ResolveCollision
//      + OnCancel
//      + OnInterrupt
//      + OnFinishCollisionResolve
//      + IsPretendingOnTile
//      + IsMakingDamageAtTile
//
//      Execution phase:
//      + PreExecutionApply
//      + OnStartExecution
//      + OnStopExecution
//      + PostExecutionApply
//
UCLASS(Blueprintable, Abstract)
class PROTOTYPE_API AAction_Base
    : public AActor
    , public ArenaStateObject_Base<State_Action_Base>
{
    CHECK_CLASS_VERSION_EQUAL(State_Action_Base, 1, 1);

    GENERATED_BODY()
        
public:

    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FEventActionStatsChanged, AAction_Base*, action);
    UPROPERTY(BlueprintAssignable, Meta = (DisplayName = "EventActionStatsChanged"))
    FEventActionStatsChanged mEventActionStatsChanged;

    DECLARE_DELEGATE_OneParam(FEventActionExecuted, AAction_Base*);
    FEventActionExecuted mEventActionExecuted;

    UFUNCTION(BlueprintPure, Category = "Action")
    static int32 MaxCancellationCounter() { return TNumericLimits<int32>().Max(); }

    UFUNCTION(BlueprintPure, Category = "Action")
    static int32 NegativeDamageRate() { return UNKNOWN_ACTION_NEGATIVE_DAMAGE_RATE; }

    UFUNCTION(BlueprintPure, Category = "Action")
    static int32 PositiveDamageRate() { return UNKNOWN_ACTION_POSITIVE_DAMAGE_RATE; }

    UFUNCTION(BlueprintPure, Category = "Action")
    static int32 NegativeStatusEffectsRate() { return UNKNOWN_ACTION_NEGATIVE_STATUS_EFFECTS_RATE; }

    UFUNCTION(BlueprintPure, Category = "Action")
    static int32 PositiveStatusEffectsRate() { return UNKNOWN_ACTION_POSITIVE_STATUS_EFFECTS_RATE; }

    UFUNCTION(BlueprintPure, Category = "Action")
    static int32 IdleActionBehavior() { return UNKNOWN_IDLE_ACTION_BEHAVIOR; }

public:

    AAction_Base(const FObjectInitializer& objectInitializer = FObjectInitializer::Get());

    virtual ~AAction_Base() = default;

    void SetActionID(uint32 ActionID);

    uint32 GetActionID() const;

    //  ~CHILD~
    //  This is useful to prepare an action for the turn, for example set some random value or so on.
    UFUNCTION(BlueprintNativeEvent, Category = "Action")
    void PrepareForPlanning();
    virtual void PrepareForPlanning_Implementation();

    //  Returns AI-hint of action usage.
    UFUNCTION(BlueprintPure, Category = "Action")
    int32 GetBehavior() const;

    //  Returns true if supports movement.
    bool DoesSupportMovement() const;

    UFUNCTION(BlueprintPure, Category = "Action")
    int32 GetMovementRange() const;

    UFUNCTION(BlueprintPure, Category = "Action")
    int32 GetMovementRangeBase() const;

    void SetMovementRangeAdditional(int32 movementRange);

    UFUNCTION(BlueprintPure, Category = "Action")
    int32 GetAdditionalMovementRange() const;

    UFUNCTION(BlueprintPure, Category = "Action")
    int32 GetInitiative() const;

    void SetAdditionalInitiative(int32 initiative);

    int32 GetAdditionalInitiative() const;

    UFUNCTION(BlueprintPure, Category = "Action")
    int32 GetRangeMin() const;

    UFUNCTION(BlueprintPure, Category = "Action")
    int32 GetRangeMax() const;

    UFUNCTION(BlueprintPure, Category = "Action")
    UTexture2D* GetIcon() const;

    UFUNCTION(BlueprintCallable, Category = "Action")
    void SetPlacement(const FIntPoint& placement);

    UFUNCTION(BlueprintPure, Category = "Action")
    FIntPoint GetPlacement() const;

    void SetExecutorActor(AActor* pExecutor);

    UFUNCTION(BlueprintPure, Category = "Action")
    AActor* GetExecutorActor() const;

    UFUNCTION(BlueprintPure, Category = "Action")
    const TScriptInterface<IArenaUnit_Interface>& GetExecutorArenaUnit() const;

    UFUNCTION(BlueprintPure, Category = "Action")
    UTiledMovement* GetExecutorTiledMovement() const;

    //  Setup the action. Executor, placement and all specific setting must be set before this call.
    UFUNCTION(BlueprintCallable, Category = "Action")
    virtual bool Setup();

    //  Manual setup:

    UFUNCTION(BlueprintPure, Category = "Action")
    bool IsManualSetupEnabled() const;

    //  Initiates manual setup process.
    UFUNCTION(BlueprintCallable, Category = "Action")
    void StartManualSetup();

    UFUNCTION(BlueprintPure, Category = "Action")
    bool IsManualSetupInProgress() const;

    //  Resets current state of the manual setup process, clears all cached data.
    UFUNCTION(BlueprintCallable, Category = "Action")
    void CancelManualSetup();

    //  Finishes setup process and returns whether the action is successfully setup or not.
    UFUNCTION(BlueprintCallable, Category = "Action")
    bool FinishManualSetup();

    //  Returns set of available for manual setup tiles, based on the last set manual setup tile.
    UFUNCTION(BlueprintPure, Category = "Action")
    const TArray<FActionDistributionTile>& GetAvailableManualSetupTiles() const;

    //  Pass a user-picked tile into "tile" to continue the manual setup process.
    //  If current AvailableManualSetupTiles contains tile, this will make step forward onto manual setup process.
    //  If tile is among previously picked tiles - the setup process will be rolled back till this tile (excluding tile itself).
    //  If tile equals executors placement - maybe it's time to cancel manual setup process?
    //  Returns whether the operation was successfull or not. Updates set of the available manual setup tiles.
    UFUNCTION(BlueprintCallable, Category = "Action")
    bool TrySetNextManualSetupTile(const FIntPoint& tile, bool& canFinishManualSetup, bool& canCancelManualSetup);

    //  Returns non-empty set of tiles If the action is setup.
    //  Array contains 1 tile if the action was setup automatically, otherwise action was setup manually.
    const TArray<FIntPoint>& GetSetupTiles() const;

    //  ~CHILD~
    //  Reset is called whenever the action should prepare itself for the reuse. It should clear setup data here.
    UFUNCTION(BlueprintNativeEvent, Category = "Action")
    void Reset();
    virtual void Reset_Implementation();

    UFUNCTION(BlueprintPure, Category = "Action")
    bool IsSetup() const;

    //  ~CLIENT~CHILD~
    //  Called whenever its needed to determine if the action can be setup at a tile.
    UFUNCTION(BlueprintPure, Category = "Action")
    virtual bool IsAvailableFor(const FIntPoint& actionPlacement) const;

    //  ~CLIENT~
    UFUNCTION(BlueprintPure, Category = "Action")
    int32 GetReloadTimer() const;

    //  
    UFUNCTION(BlueprintPure, Category = "Action")
    int32 GetLeftTurnsToReload() const;

    UFUNCTION(BlueprintPure, Category = "Action")
    bool IsReloaded() const;

    //  Can be used to force reloaded state for the action.
    UFUNCTION(BlueprintCallable, Category = "Action")
    void SetReloaded();

    //  Returns tile occupied by executor at the start of the current turn.
    UFUNCTION(BlueprintCallable, Category = "Action")
    FIntPoint GetExecutorPlacement() const;

    //  Gets distribution of the action - tiles where action should be registered.
    UFUNCTION(BlueprintCallable, Category = "Action")
    const TArray<FActionDistributionTile>& GetDistribution();

    UFUNCTION(BlueprintCallable, Category = "Action")
    int32 GetActionBehaviorsAtTile(const FIntPoint& tile) const;

    //  ~CHILD~
    //  ~!~ Call AAction_Base::StartCollisionResolve() in any child action class class.
    virtual void StartCollisionResolve(FActionCollisionResolveContext& context);

    bool IsCollisionResolveStarted() const;

    virtual FActionCollisionResolveResult ResolveCollision(FActionCollisionResolveContext& context,
                                                           AAction_Base* pLowPriorityAction,
                                                           const FIntPoint& collisionPlacement,
                                                           int32 highPrioEntryBehavior,
                                                           int32 lowPrioEntryBehavior);

    //  ~CHILD~
    //  This function is called for an action with higher priority to solve collision
    //  with lower priority action. The result of collision resolve should be written
    //  in the out-parameter.
    UFUNCTION(BlueprintNativeEvent, Category = "Action")
    FActionCollisionResolveResult ResolveCollision(AAction_Base* pLowPriorityAction,
                                                   const FIntPoint& collisionPlacement,
                                                   int32 highPrioEntryBehavior,
                                                   int32 lowPrioEntryBehavior);
    virtual FActionCollisionResolveResult ResolveCollision_Implementation(AAction_Base* pLowPriorityAction,
                                                                          const FIntPoint& collisionPlacement,
                                                                          int32 highPrioEntryBehavior,
                                                                          int32 lowPrioEntryBehavior);

    void Cancel(FActionCollisionResolveContext& context,
                AAction_Base* pCanceledBy);

    bool IncreaseCancellationCounter(FActionCollisionResolveContext& context,
                                     int32 counterChange,
                                     AAction_Base* pCancelerAction);

    bool Interrupt(FActionCollisionResolveContext& context,
                   const FIntPoint& interruptionTile,
                   AAction_Base* pInterruptedBy);

    bool IsCanceled() const;
    
    int32 GetCancellationCounterChange() const;

    int32 GetCancellationCounterLimit() const;

    bool IsLowOrderInterruptionEnabled() const;

    void UpdateReloadTimer();

    //  ~UGLY~ Should be combined with some PrepareForExecution and so on.
    void SetExecutionPriority(int32 executionPriority);


    //  Tile claim & movement:

    UFUNCTION(BlueprintPure, BlueprintNativeEvent, Category = "Action")
    bool IsClaimingTile(const FIntPoint& tile) const;
    virtual bool IsClaimingTile_Implementation(const FIntPoint& tile) const;

    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Action")
    void StopClaimingTile(const FIntPoint& tile);
    virtual void StopClaimingTile_Implementation(const FIntPoint& tile);

    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Action")
    void BlockMovement(const FIntPoint& tile,
                       bool inclusive);
    virtual void BlockMovement_Implementation(const FIntPoint& tile,
                                              bool inclusive);

    //  


    //  Finishes collision solving.
    //  For example maintanance of combat actions can be set up here, when while collision solving it
    //  was detected a few of them takes have the same victim.
    void FinishCollisionResolve();

    //  ~CHILD~
    //  ~!~ Essential parent call.
    //  Called before execution is started to apply 
    UFUNCTION(BlueprintNativeEvent, Category = "Action")
    void PreExecutionApply();
    virtual void PreExecutionApply_Implementation();

    void StartMovementExecution();

    void StopMovementExecution();

    void StartExecution();

    //  An action should call it when action's visuals are about to be finished.
    //  Can be called outside of the action if user want to skip turns visual execution.
    UFUNCTION(BlueprintCallable, Category = "Action", meta = (BlueprintProtected))
    virtual void StopExecution();

    //  ~CHILD~
    //  ~!~ Essential parent call.
    //  Called after execution being done.
    UFUNCTION(BlueprintNativeEvent, Category = "Action")
    void PostExecutionApply();
    virtual void PostExecutionApply_Implementation();

    //  ~CHILD~
    //  Override to determine whether the action pretends on occupation of a tile or not.
    UFUNCTION(BlueprintPure, BlueprintNativeEvent, Category = "Action")
    bool IsPretendingOnTile(const FIntPoint& tile) const;
    virtual bool IsPretendingOnTile_Implementation(const FIntPoint& tile) const;

    //  ~CHILD~
    //  Override to determine whether the action deals any damage at a tile or not.
    UFUNCTION(BlueprintPure, BlueprintNativeEvent, Category = "Action")
    bool IsMakingDamageAtTile(const FIntPoint& tile) const;
    virtual bool IsMakingDamageAtTile_Implementation(const FIntPoint& tile) const;

    UFUNCTION(BlueprintPure, Category = "Action")
    int32 GetExecutionPriority() const;

    UFUNCTION(BlueprintCallable, Category = "Action")
    void UpdateAvailableSetupTilesCache();

    UFUNCTION(BlueprintPure, Category = "Action")
    const TArray<FIntPoint>& GetAvailableSetupTilesCache() const;

    UFUNCTION(BlueprintPure, Category = "Action")
    void GetPotentialSetupTiles(TArray<FIntPoint>& outSetupTiles) const;

    //  Returns set of all potential available for setup tiles (pattern) based on the current executor's placement.
    UFUNCTION(BlueprintPure, Category = "Action")
    void GetSetupTilesPattern(TArray<FIntPoint>& outSetupTiles) const;

    //  Returns set of all distribution tiles (pattern) based on the current executor's placement and potential action's setup tile.
    UFUNCTION(BlueprintPure, Category = "Action")
    void GetDistributionTilesPattern(const FIntPoint& potentialSetupTile,
                                     TArray<FIntPoint>& outDistributionTiles) const;

    bool IsCombat() const;

    bool IsRange() const;

    UFUNCTION(BlueprintPure, Category = "Action")
    bool IsRecordable() const;

    int32 GetDamageRate() const;

    int32 GetStatusEffectsRate() const;

    //  ~!~?~TODO~ Determine when it is allowed to be called.
    //  Currently can be called at any time: during collision resolve and after.
    UFUNCTION(BlueprintPure, Category = "Action")
    virtual FIntPoint GetExecutorFinalTile() const;

    //  Battle Log:

    UFUNCTION(BlueprintPure, BlueprintNativeEvent, Category = "Action")
    TArray<UBattleLogItem_Base*> GenerateBattleLogItemsForCurrentTurn() const;
    virtual TArray<UBattleLogItem_Base*> GenerateBattleLogItemsForCurrentTurn_Implementation() const;

    // META:

    UFUNCTION(BlueprintPure, Category = "Action")
    const FActionCardInfo& GetActionCardInfo() const;

    void EnableSetupVisualization();

    void DisableSetupVisualization();

    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "Action")
    void StartSetupVisualization();

    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "Action")
    void StopSetupVisualization();

protected:

    //  Characteristics:

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Action")
    UTexture2D* mpIcon = nullptr;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Action", Meta = (Bitmask, BitmaskEnum = "EActionBehavior"))
    int32 mActionBehavior = 0;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Action")
    int32 mInitiativeBase = 0;

    int32 mInitiativeAdditional = 0;

    //  Describes minimal range at which action can be setup.
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Action")
    int32 mRangeMin = -1;

    //  Describes maximal range at which action can be setup.
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Action")
    int32 mRangeMax = -1;

    UPROPERTY(EditDefaultsOnly, Category = "Action")
    UActionDistributionPattern_Base* mpDistributionPattern = nullptr;

    //  Determines whether or not interrupt all lower-order entries.
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Action")
    bool mIsLowOrderInterruptionEnabled = true;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Action")
    int32 mReloadTimer = 0;

    UPROPERTY(EditDefaultsOnly, Category = "Action")
    bool mIsAOE = false;

    UPROPERTY(EditDefaultsOnly, Category = "Action")
    bool mIsCombat = false;

    UPROPERTY(EditDefaultsOnly, Category = "Action")
    bool mIsRange = false;

    UPROPERTY(EditDefaultsOnly, Category = "Action")
    bool mIsRecordable = true;

    //  Damage rate - hint for AI about how much damage an action delivers. Positive means combat actions, negative - healing, helpful:
    UPROPERTY(EditDefaultsOnly, Category = "Action")
    int32 mDamageRate = 0;

    //  Status effects rate - hint for AI about how positive are result status effects activated for target. Positive is for positive effects, negative - for negative:
    UPROPERTY(EditDefaultsOnly, Category = "Action")
    int32 mStatusEffectsRate = 0;

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Action")
    int32 mCancellationCounterChange = 1;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Action")
    int32 mCancellationCounterLimit = 10;

    // ~!~MEM~ Pay attention that this array never shrinks.
    //  ~!~TODO~ Put it into private section and provide just GetDistribution() for accessing it.
    //  It should never be modified by any child!
    UPROPERTY(BlueprintReadWrite, Category = "Action")
    TArray<FActionDistributionTile> mDistribution;

    UPROPERTY(BlueprintReadWrite, Category = "Action")
    TArray<FIntPoint> mAvailableSetupTilesCache;

    //  Meta:

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Action")
    FActionCardInfo mActionCardInfo;

    //  State:

    UPROPERTY(BlueprintReadOnly)
    FIntPoint mPlacement;

    UPROPERTY(BlueprintReadOnly)
    AActor* mpExecutorActor = nullptr;

    UPROPERTY(BlueprintReadOnly)
    UTiledMovement* mpExecutorTiledMovement = nullptr;

    UPROPERTY(BlueprintReadOnly)
    TScriptInterface<IArenaUnit_Interface> mpExecutorUnit;
    
    UPROPERTY(BlueprintReadOnly)
    bool mIsSetup = false;

    UPROPERTY(BlueprintReadOnly)
    bool mIsSetupVisualizationEnabled = false;

    UPROPERTY(BlueprintReadOnly)
    bool mIsCollisionResolveStarted = false;

    UPROPERTY(BlueprintReadOnly)
    bool mIsExecuting = false;

    UPROPERTY(BlueprintReadOnly)
    bool mIsCanceled = false;

    UPROPERTY(BlueprintReadOnly)
    int32 mLeftTurnsToReload = 0;

    int32 mCancellationCounter = 0;

    int32 mAvailableTilesCacheUpdateTimeStamp = -1;

    //  Manual setup:

    //  Determines whether manual setup is possible for the action.
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Action")
    bool mIsManualSetupEnabled = false;

protected:

    void OnConstruction(const FTransform& transform) override;

    //  Called during the construction to fill in mActionCardInfo.
    UFUNCTION(BlueprintNativeEvent, Category = "Action")
    void InitCardInfo(FActionCardInfo& cardInfo);
    virtual void InitCardInfo_Implementation(FActionCardInfo& cardInfo);

    void BeginPlay() override;

    void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    //  Planning Phase

    //  ~CHILD~
    //  Should return true if action can be setup at the specified tile at the current turn.
    //  Its used for updating available setup tiles cache. Can be called outside.
    UFUNCTION(BlueprintPure, BlueprintNativeEvent, Category = "Action")
    bool CanBeSetupAt(const FIntPoint& potentialSetupTile) const;
    virtual bool CanBeSetupAt_Implementation(const FIntPoint& potentialSetupTile) const;

    //  ~CHILD~
    //  Called during setup process to determine whether tile should be included into distribution.
    //  ~!~ Action can store some setup-specific stuff to be used during the execution e.g. victim units.
    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Action")
    bool TryDistributeAt(const FIntPoint& potentialDistributionTile, int32& outBehaviorAtDistributionTile);
    virtual bool TryDistributeAt_Implementation(const FIntPoint& potentialDistributionTile, int32& outBehaviorAtDistributionTile);

    //  ~CHILD~
    //  Called after all distribution tiles are processed. It is the last chance to cancel setup.
    //  Return value indicates whether setup process has succeeded or not.
    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Action")
    bool FinalizeSetupProcess();
    virtual bool FinalizeSetupProcess_Implementation();

    //  Action Collision Resolve Phase

    virtual void OnCancel(FActionCollisionResolveContext& context, 
                          AAction_Base* pCancelerAction);

    //  ~CHILD~
    //  Called when the action is canceled.
    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Action")
    void OnCancel(AAction_Base* pCancelerAction);
    virtual void OnCancel_Implementation(AAction_Base* pCancelerAction);

    virtual bool OnInterrupt(FActionCollisionResolveContext& context, 
                             const FIntPoint& interruptionTile,
                             AAction_Base* pInterrupterAction);

    //  ~CHILD~
    //  Called when the action is interrupted at a tile. The return value indicates if action should be totally canceled.
    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Action")
    bool OnInterrupt(const FIntPoint& interruptionTile,
                     AAction_Base* pInterrupterAction);
    virtual bool OnInterrupt_Implementation(const FIntPoint& interruptionTile,
                                            AAction_Base* pInterrupterAction);

    //  ~CHILD~
    //  Called after all collisions are resolved. Here action can summarize the result
    //  of all collisions in which it took part and react to it, e.g. cancel itself.
    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Action")
    void OnFinishCollisionResolve();
    virtual void OnFinishCollisionResolve_Implementation();

    //  Turn Execution Phase

    //  ~CHILD~
    //  Called when action visuals (animations, VFX) should start working. No factual action influance should be implemented here.
    UFUNCTION(BlueprintNativeEvent, Category = "Action")
    void OnStartExecution();
    virtual void OnStartExecution_Implementation();

    //  ~CHILD~
    //  Called when action visuals (animations, VFX) should stop working and go to the default idle pose. No factual action influance should be implemented here.
    UFUNCTION(BlueprintNativeEvent, Category = "Action")
    void OnStopExecution();
    virtual void OnStopExecution_Implementation();

    //  Refills mAvailableManualSetupTiles with a new set of available for user selection tiles based on the baseManualSetupTile.
    void UpdateAvailableManualSetupTiles();

    //  Generate set of available for manual setup tiles based on the base manual setup tile.
    virtual void GenerateAvailableManualSetupTiles(const FIntPoint& baseManualSetupTile, TArray<FActionDistributionTile>& outAvailableManualSetupTiles);

    virtual void EnableSetupVisualization_Inner();

    virtual void DisableSetupVisualization_Inner();

    //  Called when action setup visualization is enabled and action is either reset or resetup.
    virtual void UpdateSetupVisualization();

#if WITH_EDITOR

    void PostEditChangeProperty(FPropertyChangedEvent& propertyChangedEvent);

#endif // #if WITH_EDITOR

private:

    UPROPERTY(Replicated)
    uint32 mActionID = 0;

    int32 mExecutionPrioirty;

    bool mIsManualSetupInProgress = false;

    //  Set of tiles which are available to be picked by the user as the next manual setup tile.
    //  Changes each time user successfully picks another next manual setup tile.
    //  FActionDistributionTile allows to show the user potential behavior at the specified tile.
    TArray<FActionDistributionTile> mAvailableManualSetupTiles;

    //  Set of tiles which were used to setup the action. Must be replicated to other clients to replicate setup.
    //  1 tile = auto setup. > 1 tiles = manual setup.
    TArray<FIntPoint> mSetupTiles;

private:

    void StoreState_Inner(ArenaStateContext& context, 
                          State_Action_Base& state) const override;

    bool RestoreState_Inner(const State_Action_Base& state) override;

    //
    //  Complex actions:
    //

public:

    UFUNCTION(BlueprintCallable)
    void SetMovementPath(const TArray<FIntPoint>& pathTiles);

    UFUNCTION(BlueprintCallable)
    void SetExecutionDirection(int32 tileDirection);

    void SetActionSetupData(const FActionSetupData& setupData);

    const FActionSetupData& GetActionSetupData() const;

    void SetFinalTile(const FIntPoint& tile);

    //  Returns tile where an exeuctor will be placed after movement conflicts being resolved.
    FIntPoint GetFinalTile() const;

protected:

    UPROPERTY(BlueprintReadOnly)
    FActionSetupData mSetupData;

    //  Tile where an executor will be placed after all movement conflicts being resolved.
    UPROPERTY(BlueprintReadOnly)
    FIntPoint mFinalTile;

private:

    //  Max range of the movement. May be set to 0 if movement is disallowed.
    UPROPERTY(EditDefaultsOnly, Category = "Action", Meta = (DisplayName = "Movement Range (base)"))
    int32 mMovementRangeBase = 0;

    int32 mMovementRangeAdditional = 0;

};
