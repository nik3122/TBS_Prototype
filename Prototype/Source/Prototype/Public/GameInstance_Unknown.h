// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Net/UnrealNetwork.h"
#include "Online.h"
#include "GameProgressSaveGame.h"
#include "Arena/CreatureBase.h"
#include "Arena/Weather.h"
#include "DialogueSystem.h"
#include "ActionScript_LoadArena.h"
#include "GlobalMapState.h"
#include "Arena/ArenaAIConfig.h"
#include "CampaignSaveGame.h"
#include "Arena/ArenaResults.h"
#include "Arena/ArenaSetupInfo.h"
#include "GameInstance_Unknown.generated.h"


class UProgressManager;
struct FActionScriptData;


USTRUCT(BlueprintType)
struct FUnknownOnlineSessionInfo
{
    GENERATED_BODY()

public:

    UPROPERTY(BlueprintReadWrite, Category = "Unknown")
    int32 mPingInMs = -1;

    UPROPERTY(BlueprintReadWrite, Category = "Unknown")
    FString mSessionName;

    UPROPERTY(BlueprintReadWrite, Category = "Unknown")
    FString mHostID;

    UPROPERTY(BlueprintReadWrite, Category = "Unknown")
    FString mHostName;

    UPROPERTY(BlueprintReadWrite, Category = "Unknown")
    FString mMapName;

    UPROPERTY(BlueprintReadWrite, Category = "Unknown")
    FString mServerName;

    UPROPERTY(BlueprintReadWrite, Category = "Unknown")
    int32 mMaxPlayersNum = 0;

    UPROPERTY(BlueprintReadWrite, Category = "Unknown")
    int32 mCurrentPlayersNum = 0;
};


UENUM(BlueprintType)
enum class EGameStage : uint8
{
    GameStage_MainMenu,
    GameStage_GlobalMap,
    GameStage_Arena,
};


UCLASS(Category = "Unknown")
class PROTOTYPE_API UGameInstance_Unknown
    : public UGameInstance
{
	GENERATED_BODY()

public:

    bool mLoadGlobalMapState;

    FGlobalMapState mGlobalMapState;

    FArenaResultScenario mArenaResultScenario;

    //  Arena transition:

    FString mAfterArenaLevelName;

    TArray<TSubclassOf<ACreatureBase>> mPlayerPartyMembers;

    TArray<TSubclassOf<ACreatureBase>> mAIPartyMembers;

    FWeatherState mWeatherState;

    FArenaResults mArenaResults;

    FArenaAIConfig mArenaAIConfig;

    //  Campaign data:

    UPROPERTY(BlueprintReadOnly, Category = "Unknown")
    UProgressManager* mpProgressManager;

    //  Common game data:

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Unknown")
    UDataTable* mpCreaturesInfoTable;

    UPROPERTY(BlueprintReadOnly, Category = "Unknown")
    UGameProgressSaveGame* mpGameProgressSaveGame;

    UPROPERTY()
    UCampaignSaveGame* mpCampaignSaveGame;

    //UPROPERTY(BlueprintReadOnly, Category = "Unknown")
    //EGameStage mGameStage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unknown")
    FArenaSetupInfo mArenaSetupInfo;

    //  Constant core levels:

    UPROPERTY(EditDefaultsOnly, Category = "Unknown")
    FName mArenaLobbyLevel;

    UPROPERTY(EditDefaultsOnly, Category = "Unknown")
    FName mGlobalMapLobbyLevel;

    UPROPERTY(EditDefaultsOnly, Category = "Unknown")
    FName mMainMenuLevel;

    //  Menu:

    UPROPERTY(BlueprintReadWrite, Category = "Unknown")
    bool mRestoreMainMenuStack = false;

    UPROPERTY(BlueprintReadWrite, Category = "Unknown")
    TArray<TSubclassOf<UUserWidget>> mMainMenuStack;

public:

    UGameInstance_Unknown(const FObjectInitializer& objectInitializer = FObjectInitializer::Get());

    UFUNCTION(BlueprintPure, Category = "Unknown")
    UProgressManager* GetProgressManager() const;

    void EnqueueActionScript(const FActionScriptData& actionScriptData);

    bool DequeueActionScript(FActionScriptData& outActionScriptData);

    UFUNCTION(BlueprintCallable, Category = "Unknown")
    void StartNewCampaign(const FName& globalMapLevelName);

    /*
        ~!~ SaveCampaign is available only on Global Map levels.
    */
    UFUNCTION(BlueprintCallable, Category = "Unknown")
    bool SaveCampaign(const FSaveSlot& saveSlot);

    UFUNCTION(BlueprintCallable, Category = "Unknown")
    bool LoadCampaign(const FSaveSlot& saveSlot);

    //UFUNCTION(BlueprintCallable, Category = "Unknown")
    //void StartArena(const FName& afterArenaLevelName, const FName& arenaLevelName);

    //void FinishArena();

    //  Helpers:

    UFUNCTION(BlueprintPure, Category = "Unknown")
    static FString DateTimeToString(const FDateTime& dateTime);

    UFUNCTION(BlueprintPure, Category = "Unknown")
    static FString DateTimeToStringFormat(const FDateTime& dateTime,
                                          const FString& format);


protected:

    UPROPERTY(EditDefaultsOnly)
    FName mMainMenuLevelName;

private:

    TQueue<FActionScriptData> mActionScripts;

    const FString mUnknownSaveGameSlot;

private:

    void Init() override;

    void Shutdown() override;

    void LoadGameProgress();

    void SaveGameProgress();

    //////////////////////////////////////////////////////////////////////////
    //                              NETWORK
    //////////////////////////////////////////////////////////////////////////

public:

    DECLARE_DYNAMIC_DELEGATE_OneParam(FEventStartHostSessionCompleted, bool, isSuccessful);

    DECLARE_DYNAMIC_DELEGATE(FEventStartSessionsSearchCompleted);

    DECLARE_DYNAMIC_DELEGATE_OneParam(FEventStartJoinSessionCompleted, FText, reasonMessage);

    DECLARE_DYNAMIC_DELEGATE_OneParam(FEventShutDownCurrentSessionCompleted, bool, isSuccessful);

    UPROPERTY(BlueprintReadOnly, Category = "Unknown")
    TArray<FUnknownOnlineSessionInfo> mFoundOnlineSessions;

    UPROPERTY(BlueprintReadOnly, Category = "Unknown")
    FString mMyNetworkAddress;

public:

    UFUNCTION(BlueprintPure, Category = "Unknown")
    FString GetMyIP() const;

    UFUNCTION(BlueprintCallable, Category = "Unknown")
    void LogCurrentSessionState();

    UFUNCTION(BlueprintCallable, Category = "Unknown")
    void OpenMainMenu(bool restoreLastMenuStack);

    UFUNCTION(BlueprintCallable, Category = "Unknown")
    void OpenArenaLobby(bool isLAN, const FString& sessionName);

    UFUNCTION(BlueprintCallable, Category = "Unknown")
    void OpenArena(FName arenaLevelName);

    UFUNCTION(BlueprintCallable, Category = "Unknown")
    bool StartHostSession(int32 maxPlayersNum, bool isLAN, const FString& serverName,
                          const FEventStartHostSessionCompleted& onStartHostSessionCompleted);

    UFUNCTION(BlueprintCallable, Category = "Unknown")
    bool StartSessionSearch(bool isLAN, const FEventStartSessionsSearchCompleted& onStartSessionsSearchCompleted);

    UFUNCTION(BlueprintCallable, Category = "Unknown")
    bool StartJoinSession(int32 foundSessionIndex, const FEventStartJoinSessionCompleted& onStartJoinSessionCompleted);

    /*
        Return value: 
    */
    UFUNCTION(BlueprintCallable, Category = "Unknown")
    bool ShutDownCurrentSession(const FEventShutDownCurrentSessionCompleted& onShutDownCurrentSessionCompleted);

protected:

    FOnCreateSessionCompleteDelegate mEventCreateSessionCompleted;
    FDelegateHandle mEventCreateSessionCompletedHandle;

    FOnStartSessionCompleteDelegate mEventStartSessionCompleted;
    FDelegateHandle mEventStartSessionCompletedHandle;

    FOnFindSessionsCompleteDelegate mEventFindSessionsCompleted;
    FDelegateHandle mEventFindSessionsCompletedHandle;

    FOnJoinSessionCompleteDelegate mEventJoinSessionCompleted;
    FDelegateHandle mEventJoinSessionCompletedHandle;

    FOnDestroySessionCompleteDelegate mEventDestroySessionCompleted;
    FDelegateHandle mEventDestroySessionCompletedHandle;

    TSharedPtr<FOnlineSessionSettings> mpSessionSettings;

    TSharedPtr<FOnlineSessionSearch> mpSessionSearch;

protected:

    UFUNCTION()
    void OnArenaLobbySessionHosted(bool isSuccessful);

    UFUNCTION()
    void OnShutDownCurrentSession(bool isSuccessful);

private:

    FEventStartHostSessionCompleted mEventStartHostSessionCompleted;

    FEventStartSessionsSearchCompleted mEventStartSessionsSearchCompleted;

    FEventStartJoinSessionCompleted mEventStartJoinSessionCompleted;

    FEventShutDownCurrentSessionCompleted mEventShutDownCurrentSessionCompleted;

private:

    bool TryHostSession(TSharedPtr<const FUniqueNetId> userId, FName sessionName, bool isLAN, bool isPresence, int32 maxPlayersNum, const FString& serverName);

    bool TryFindSessions(TSharedPtr<const FUniqueNetId> userId, bool isLAN, bool isPresence, int32 maxSearchResultsNum);

    bool TryJoinSession(TSharedPtr<const FUniqueNetId> userId, FName sessionName, const FOnlineSessionSearchResult& searchResult);

    bool TryDestroySession(FName sessionName);

    void OnCreateSessionCompleted(FName sessionName, bool isSuccessful);

    void OnStartSessionCompleted(FName sessionName, bool isSuccessful);

    void OnFindSessionsCompleted(bool isSuccessful);

    void OnJoinSessionCompleted(FName sessionName, EOnJoinSessionCompleteResult::Type joinResult);

    void OnDestroySessionCompleted(FName sessionName, bool isSuccessful);

    void OnNetworkFailure(UWorld* pWorld, UNetDriver* pNetDriver, ENetworkFailure::Type failureType, const FString& errorString);
};
