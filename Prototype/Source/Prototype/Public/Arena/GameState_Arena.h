#pragma once


#include "CoreMinimal.h"
#include "Arena/ArenaSetupInfo.h"
#include "Arena/ArenaResults.h"
#include "Arena/ArenaManager_Interface.h"
#include "Arena/ArenaState.h"
#include "Arena/CreatureBase.h"
#include "Arena/FireSystem.h"
#include "Arena/PassMap.h"
#include "GameFramework/GameStateBase.h"
#include "GameState_Arena.generated.h"


class AGameMode_ArenaFight;
class AArenaManager;
class APlayerState_Arena;


/*
    Global state of arena.
*/
struct State_Arena
{
    DECLARE_CLASS_VERSION(1, 0);

    int32 mTurn = -1;
    State_CreatureBase** mppPlayerPartyMembersStates = nullptr;
    int32 mPlayerPartyMembersStatesCount = 0;
    State_CreatureBase** mppAIPartyMembersStates = nullptr;
    int32 mAIPartyMembersStatesCount = 0;
    State_FireSystem* mpFireSystemState = nullptr;
    State_PassMap* mpPassMapState = nullptr;
};


/*
    BugReportInfo contains data which should be included into report.
*/
USTRUCT(BlueprintType)
struct FBugReportInfo
{
    GENERATED_BODY()

public:

    UPROPERTY(BlueprintReadWrite, Category = "Arena")
    FString mReportName;

    UPROPERTY(BlueprintReadWrite, Category = "Arena")
    FString mCustomInfo;
};


UCLASS()
class AGameState_Arena
    : public AGameStateBase
    , public ArenaStateObject_Base<State_Arena>
{
    CHECK_CLASS_VERSION_EQUAL(State_Arena, 1, 0);

    GENERATED_BODY()

    friend AGameMode_ArenaFight;

public:
    /*
    DECLARE_DYNAMIC_MULTICAST_DELEGATE(FEventSystemsSetup);
    UPROPERTY(BlueprintAssignable, Category = "Arena")
    FEventSystemsSetup mEventSystemsSetup;
    */
    /*
        In case of DRAW winnerPartyIndex < 0 otherwise winnerPartyIndex >= 0.
    */
    DECLARE_DYNAMIC_MULTICAST_DELEGATE(FEventEndOfArena);
    UPROPERTY(BlueprintAssignable, Category = "Arena")
    FEventEndOfArena mEventEndOfArena;

public:

    UPROPERTY(BlueprintReadOnly, Category = "Arena")
    FArenaResults mArenaResults;

    UPROPERTY(BlueprintReadOnly, Replicated, Category = "Arena")
    APlayerState_Arena* mpPlayerStateOne = nullptr;

    UPROPERTY(BlueprintReadOnly, Replicated, Category = "Arena")
    APlayerState_Arena* mpPlayerStateTwo = nullptr;

    UPROPERTY(BlueprintReadOnly, Category = "Arena")
    bool mIsBattleStarted = false;

    UPROPERTY(BlueprintReadOnly, Category = "Arena")
    bool mIsBattleFinished = false;

public:

    AArenaManager* GetArenaManager() const;

    void EndOfArena(const FArenaResults& arenaResults);

    UFUNCTION(BlueprintPure, Category = "Arena")
    EArenaGameMode GetArenaGameMode() const;

    UFUNCTION(BlueprintPure, Category = "Arena")
    APlayerState_Arena* GetMyPlayerState() const;

    UFUNCTION(BlueprintPure, Category = "Arena")
    APlayerState_Arena* GetOpponentPlayerState() const;

    UFUNCTION(BlueprintCallable, Category = "Arena")
    void SaveBugReport(const FBugReportInfo& bugReportInfo);

    //  ArenaStateObject_Base::

    void StoreState_Inner(ArenaStateContext& context, State_Arena& state) const override;

    bool RestoreState_Inner(const State_Arena& state) override;

protected:

    UPROPERTY(Replicated)
    EArenaGameMode mArenaGameMode = EArenaGameMode::INVALID;

    UPROPERTY()
    AArenaManager* mpArenaManager = nullptr;

protected:

    void BeginPlay() override;

    void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    bool HasMatchStarted() const override;

    UFUNCTION(NetMulticast, Reliable)
    void Multicast_EndOfArena(FArenaResults arenaResults);

    UFUNCTION()
    void OnSystemsSetup();

private:

    ArenaStateContext mArenaStateContextTurnInitial;
    State_Arena* mpArenaStateTurnInitial = nullptr;
    ArenaStateContext mArenaStateContextTurnResolved;
    State_Arena* mpArenaStateTurnResolved = nullptr;
    ArenaStateContext mArenaStateContextTurnFinal;
    State_Arena* mpArenaStateTurnFinal = nullptr;

private:

    void OnStoreTurnStateInitial();

    void OnStoreTurnStateResolved();

    void OnStoreTurnStateFinal();

    void SaveTurnStateToFile();

};