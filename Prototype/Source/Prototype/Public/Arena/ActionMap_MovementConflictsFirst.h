#pragma once


#include "CoreMinimal.h"
#include "Arena/ActionMap.h"
#include "Arena/ArenaUnit_Interface.h"
#include "Arena/Damage.h"
#include "Arena/TiledMovement.h"
#include "ActionMap_MovementConflictsFirst.generated.h"


class APassMap;
class UBattleLogItem_Base;
class UBattleLogItem_PushCreature;
class UBattleLogItem_RamCreature;


UCLASS()
class AActionMap_MovementConflictsFirst
    : public AActionMap
{
    GENERATED_BODY()

public:

    void BeginPlay() override;

    void Tick(float deltaSconds) override;

    void SetupSystem(ATurnsManager* turnsManager) override;

protected:

    int32 CalculateActionPriority(AAction_Base* action) override;

    void SubmitAction_Inner(AAction_Base* action,
                            int32 executionPriority) override;

    void ResolveActionConflicts_Inner() override;

    void OnApplyActionsPreExecution() override;

    void OnStartExecution() override;

    void OnStopExecution() override;

    void OnPostExecution() override;

private:

    enum class EMovementEventType
    {
        Move,           //  Simply move arena unit by specified path (with no conflicts).
        MoveAndPush,    //  Move arena unit by specified path and push another arena unit specified in the next following Push event.
        Push,           //  Describes arena unit affected by the previous MoveAndPush event. mPath contains tile at which mArenaUnit have to be pushed.
        MoveAndRam,     //  Move arena unit by specified path and ram another arena unit specified in the next following Ram event.
        Ram,            //  Describes arena unit affected by the previous MoveAndRam event.
    };

    struct FMovementEvent
    {
        EMovementEventType mType;
        TScriptInterface<IArenaUnit_Interface> mArenaUnit;
        TWeakObjectPtr<AAction_Base> mAction;
        TArray<FIntPoint> mPath;
        UBattleLogItem_Base* mBattleLogItem = nullptr;
        bool mHasConflict = false;
    };

    DECLARE_DYNAMIC_DELEGATE(FDelegateMovementFinished);

private:

    UPROPERTY()
    APassMap* mPassMap = nullptr;

    TArray<FMovementEvent> mMovementEvents;

    int32 mExecutingMovementEventIndex = INDEX_NONE;

    int32 mExecutingActionIndex = INDEX_NONE;

    TMap<UTiledMovement*, FDelegateHandle> mOnMovementFinishedDelegates;

    FDelegateHandle mOnMovementFinished;

    FDamageParameters mPushDamage;

    FDamageParameters mRamDamage;

    //  ~TEMP~ Push event:
    AActor* mActorToPush = nullptr;
    FIntPoint mPushTile;
    FVector mPushStartLocation;
    FVector mPushEndLocation;
    float mPushElapsedTime = 0.f;
    float mPushDuration = 0.4f;

    //  Used to track executing non-conflict move events at the given moment.
    int32 mCountOfExecutingNonConflictMoveEvents = 0;

private:

    //  Searching for the deepest tile in the path which can be occupied with or without ram/push.
    //  All obstacles on the path afore the found tile are ignored.
    //  Generates movement events queue.
    void ResolveMovementConflicts_IgnoreIntermediateObstacles(TMap<AAction_Base*, FIntPoint>& outFinalTiles);

    //  Searching the first tile in the path which is not an obstacle or which can be rammed.
    //  Generates movement events queue.
    void ResolveMovementConflicts_StopAtClosestObstacle(TMap<AAction_Base*, FIntPoint>& outFinalTiles);

    void ExecuteMoveEvent(const FMovementEvent& moveEvent,
                          const UTiledMovement::FDelegateMovementStopped& delegate);

    void ExecuteMoveAndPushEvent(const FMovementEvent& moveAndPushEvent,
                                 const FMovementEvent& pushEvent);

    void ExecuteMoveAndRamEvent(const FMovementEvent& moveAndRamEvent,
                                const FMovementEvent& ramEvent);

    UFUNCTION()
    void OnNonConflictMovementFinished(UTiledMovement* passMapMovement);

    UFUNCTION()
    void OnConflictMovementFinished(UTiledMovement* passMapMovement);

    void OnMovementFinished(UTiledMovement* passMapMovement);

    void StartPush(AActor* actorToPush,
                   const FIntPoint& pushTile);

    void OnPushFinished();

    void OnNonConflictMovementEventExecuted();

    void StartExecutingConflictMovementEvents();

    void OnConflictMovementEventExecuted();

    void OnAllConflictMovementEventsExecuted();

    void AffectPushVictim(AActor* pushCauserArenaUnit,
                          AActor* pushVictimArenaUnit,
                          UBattleLogItem_PushCreature* resultBattleLogItem,
                          bool& outIsVictimKilled);

    void AffectRamVictim(AActor* ramCauserArenaUnit,
                         AActor* ramVictimArenaUnit,
                         UBattleLogItem_RamCreature* resultBattleLogItem,
                         bool& outIsVictimKilled);

    void AffectPushVictimVisual(AActor* pushCauserArenaUnit,
                                AActor* pushVictimArenaUnit);

    void AffectRamVictimVisual(AActor* ramCauserArenaUnit,
                               AActor* ramVictimArenaUnit);

    void OnActionExecuted(AAction_Base* executedAction);

    void OnAllActionsExecuted();
};