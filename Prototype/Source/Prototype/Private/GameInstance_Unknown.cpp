// Fill out your copyright notice in the Description page of Project Settings.

#include "GameInstance_Unknown.h"
#include "UnknownCommon.h"
#include "GameMode_GlobalMap.h"
#include "Arena/GameMode_ArenaFight.h"
#include "ProgressManager.h"
#include "SocketSubsystem.h"
#include "Kismet/GameplayStatics.h"


UGameInstance_Unknown::UGameInstance_Unknown(const FObjectInitializer& objectInitializer)
    : Super(objectInitializer)
    , mLoadGlobalMapState(true)
    , mUnknownSaveGameSlot(TEXT("GameProgress"))
    //, mGameStage(EGameStage::GameStage_MainMenu)
{
    mEventCreateSessionCompleted = FOnCreateSessionCompleteDelegate::CreateUObject(this, &UGameInstance_Unknown::OnCreateSessionCompleted);
    mEventStartSessionCompleted = FOnStartSessionCompleteDelegate::CreateUObject(this, &UGameInstance_Unknown::OnStartSessionCompleted);
    mEventFindSessionsCompleted = FOnFindSessionsCompleteDelegate::CreateUObject(this, &UGameInstance_Unknown::OnFindSessionsCompleted);
    mEventJoinSessionCompleted = FOnJoinSessionCompleteDelegate::CreateUObject(this, &UGameInstance_Unknown::OnJoinSessionCompleted);
    mEventDestroySessionCompleted = FOnDestroySessionCompleteDelegate::CreateUObject(this, &UGameInstance_Unknown::OnDestroySessionCompleted);

}

void UGameInstance_Unknown::Init()
{
    Super::Init();

    //  ~?~ Possibly some flags are needed (e.g. RF_RootSet or RF_Standalone):
    mpProgressManager = NewObject<UProgressManager>(this);

    LoadGameProgress();

    mpCampaignSaveGame = Cast<UCampaignSaveGame>(UGameplayStatics::CreateSaveGameObject(UCampaignSaveGame::StaticClass()));

    GetEngine()->OnNetworkFailure().AddUObject(this, &UGameInstance_Unknown::OnNetworkFailure);
}

UProgressManager* UGameInstance_Unknown::GetProgressManager() const
{
    return mpProgressManager;
}

void UGameInstance_Unknown::EnqueueActionScript(const FActionScriptData& actionScriptData)
{
    mActionScripts.Enqueue(actionScriptData);
}

bool UGameInstance_Unknown::DequeueActionScript(FActionScriptData& outActionScriptData)
{
    return mActionScripts.Dequeue(outActionScriptData);
}

void UGameInstance_Unknown::Shutdown()
{
    Super::Shutdown();

    ShutDownCurrentSession(FEventShutDownCurrentSessionCompleted());
}

void UGameInstance_Unknown::StartNewCampaign(const FName& globalMapLevelName)
{
    //  Starting new campaign we have to do the next steps:
    //  1) Reset progress manager.
    //  2) Reset campaign game save.
    //  3) Open global map level of the new campaign.
    mpProgressManager->ResetProgress();
    mpCampaignSaveGame->ResetState();
    UGameplayStatics::OpenLevel(this, globalMapLevelName);
}

bool UGameInstance_Unknown::SaveCampaign(const FSaveSlot& saveSlot)
{
    if (mActionScripts.IsEmpty())
    {
        AGameMode_GlobalMap* pGlobalMapGameMode = UUnknownCommon::GetGlobalMapGameMode(this);
        if (pGlobalMapGameMode != nullptr &&
            pGlobalMapGameMode->CanSaveCampaignNow())
        {
            pGlobalMapGameMode->SaveState();
            //  ~!~ As the owner we save progress manager's state:
            mpProgressManager->SaveState(mpCampaignSaveGame->mProgressManagerState);
            const_cast<FSaveSlot&>(saveSlot).mSaveDate = FDateTime::Now();
            if (UGameplayStatics::SaveGameToSlot(mpCampaignSaveGame, saveSlot.mSlotName, saveSlot.mUserIndex))
            {
                mpGameProgressSaveGame->mSaveSlots.RemoveSingle(saveSlot);
                mpGameProgressSaveGame->mSaveSlots.Add(saveSlot);
                SaveGameProgress();
                return true;
            }
        }
    }
    return false;
}

bool UGameInstance_Unknown::LoadCampaign(const FSaveSlot& saveSlot)
{
    if (UGameplayStatics::DoesSaveGameExist(saveSlot.mSlotName, saveSlot.mUserIndex))
    {
        mpCampaignSaveGame = Cast<UCampaignSaveGame>(UGameplayStatics::LoadGameFromSlot(saveSlot.mSlotName, 0));
        if (mpCampaignSaveGame != nullptr)
        {
            //  ~!~ As the owner we load progress manager's state:
            mpProgressManager->LoadState(mpCampaignSaveGame->mProgressManagerState);
            //  ~!~ Actions scripts should be cleared before loading a campaign:
            mActionScripts.Empty();
            //mGameStage = EGameStage::GameStage_GlobalMap;
            UGameplayStatics::OpenLevel(this, saveSlot.mGlobalMapLevelName);
            return true;
        }
    }
    return false;
}

void UGameInstance_Unknown::LoadGameProgress()
{
    mpGameProgressSaveGame = Cast<UGameProgressSaveGame>(UGameplayStatics::LoadGameFromSlot(mUnknownSaveGameSlot, 0));
    /*
        If no GameProgress save exists - first game start, we should create game progress on our own:
    */
    if (mpGameProgressSaveGame == nullptr)
    {
        mpGameProgressSaveGame = Cast<UGameProgressSaveGame>(UGameplayStatics::CreateSaveGameObject(UGameProgressSaveGame::StaticClass()));
    }
    else
    {
        mpGameProgressSaveGame->mSaveSlots.RemoveAll([](const FSaveSlot& saveSlot){
            return UGameplayStatics::DoesSaveGameExist(saveSlot.mSlotName, saveSlot.mUserIndex) == false;
        });
    }
}

void UGameInstance_Unknown::SaveGameProgress()
{
    if (mpGameProgressSaveGame != nullptr)
    {
        //  ~!~BUG~ Default 0 user index can cause a bug.
        UGameplayStatics::SaveGameToSlot(mpGameProgressSaveGame, mUnknownSaveGameSlot, 0);
    }
}

FString UGameInstance_Unknown::DateTimeToString(const FDateTime& dateTime)
{
    return dateTime.ToString();
}

FString UGameInstance_Unknown::DateTimeToStringFormat(const FDateTime& dateTime,
                                                      const FString& format)
{
    return dateTime.ToString(*format);
}

/*void UGameInstance_Unknown::StartArena(const FName& afterArenaLevelName, const FName& arenaLevelName)
{
    AGameModeBase* pGameMode = UGameplayStatics::GetGameMode(this);
    AGameMode_GlobalMap* pGameModeGlobalMap = Cast<AGameMode_GlobalMap>(pGameMode);
    if (pGameModeGlobalMap != nullptr)
    {
        pGameModeGlobalMap->SaveState();
    }
    mAfterArenaLevelName = afterArenaLevelName;
    mGameStage = EGameStage::GameStage_Arena;
    UGameplayStatics::OpenLevel(this, arenaLevelName);
}

void UGameInstance_Unknown::FinishArena()
{
    if (mAfterArenaLevelName.IsNone())
    {
#if WITH_EDITOR
        //  ~!~TODO~ If PIE - leave PIE.
        UGameplayStatics::GetPlayerController(this, 0)->ConsoleCommand("quit");
#endif // #if WITH_EDITOR
    }
    else
    {
        UGameplayStatics::OpenLevel(this, mAfterArenaLevelName);
    }
}*/


//////////////////////////////////////////////////////////////////////////
//                              NETWORK
//////////////////////////////////////////////////////////////////////////

FString UGameInstance_Unknown::GetMyIP() const
{
    ISocketSubsystem* pSocketSub = ISocketSubsystem::Get();
    if (pSocketSub)
    {
        bool canBuildAll = false;
        TSharedPtr<FInternetAddr> pLocalHostAddr = pSocketSub->GetLocalHostAddr(*GLog, canBuildAll);
        if (pLocalHostAddr)
        {
            return pLocalHostAddr->ToString(false);
        }
    }
    return "invalid_addr";
}

void UGameInstance_Unknown::LogCurrentSessionState()
{
    IOnlineSubsystem* pOnlineSub = IOnlineSubsystem::Get();
    if (pOnlineSub)
    {
        IOnlineSessionPtr pSession = pOnlineSub->GetSessionInterface();
        if (pSession.IsValid())
        {
            FString stateText;
            switch (pSession->GetSessionState(GameSessionName))
            {
            case EOnlineSessionState::Destroying:
                stateText = "Destroying";
                break;
            case EOnlineSessionState::Ended:
                stateText = "Ended";
                break;
            case EOnlineSessionState::Ending:
                stateText = "Ending";
                break;
            case EOnlineSessionState::InProgress:
                stateText = "InProgress";
                break;
            case EOnlineSessionState::NoSession:
                stateText = "NoSession";
                break;
            case EOnlineSessionState::Pending:
                stateText = "Pending";
                break;
            case EOnlineSessionState::Starting:
                stateText = "Starting";
                break;
            }
            stateText = "Current session state: %s" + stateText;
            UE_LOG(UnknownCommonLog, Log, TEXT("Current session state: %s"), *stateText);
            GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::White, stateText);
        }
    }
}

void UGameInstance_Unknown::OpenMainMenu(bool restoreLastMenuStack)
{
    LogCurrentSessionState();
    mRestoreMainMenuStack = restoreLastMenuStack;
    FEventShutDownCurrentSessionCompleted completedDelegate;
    completedDelegate.BindDynamic(this, &UGameInstance_Unknown::OnShutDownCurrentSession);
    if (ShutDownCurrentSession(completedDelegate) == false)
    {
        UGameplayStatics::OpenLevel(GetWorld(), mMainMenuLevel, true);
    };
}

void UGameInstance_Unknown::OpenArenaLobby(bool isLAN, const FString& sessionName)
{
    LogCurrentSessionState();
    FEventStartHostSessionCompleted completedDelegate;
    completedDelegate.BindDynamic(this, &UGameInstance_Unknown::OnArenaLobbySessionHosted);
    if (StartHostSession(2, isLAN, sessionName, completedDelegate) == false)
    {
        UE_LOG(UnknownCommonLog, Warning, TEXT("Cannot start online session."));
    };
}

void UGameInstance_Unknown::OpenArena(FName arenaLevelName)
{
    FString url = arenaLevelName.ToString();
    GetWorld()->ServerTravel(url);
}

bool UGameInstance_Unknown::StartHostSession(int32 maxPlayersNum, bool isLAN, const FString& serverName,
                                             const FEventStartHostSessionCompleted& onHostSessionCompleted)
{
    mEventStartHostSessionCompleted = onHostSessionCompleted;
    TSharedPtr<const FUniqueNetId> userId = GetFirstGamePlayer()->GetPreferredUniqueNetId().GetUniqueNetId();
    return TryHostSession(userId, GameSessionName, isLAN, false, maxPlayersNum, serverName);
}

bool UGameInstance_Unknown::StartSessionSearch(bool isLAN, const FEventStartSessionsSearchCompleted& onStartSessionsSearchCompleted)
{
    mEventStartSessionsSearchCompleted = onStartSessionsSearchCompleted;
    TSharedPtr<const FUniqueNetId> userId = GetFirstGamePlayer()->GetPreferredUniqueNetId().GetUniqueNetId();
    int32 maxSearchResultNum = 20;
    return TryFindSessions(userId, isLAN, false, maxSearchResultNum);
}

bool UGameInstance_Unknown::StartJoinSession(int32 foundSessionIndex, const FEventStartJoinSessionCompleted& onStartJoinSessionCompleted)
{
    mEventStartJoinSessionCompleted = onStartJoinSessionCompleted;
    if (mpSessionSearch->SearchResults.IsValidIndex(foundSessionIndex))
    {
        TSharedPtr<const FUniqueNetId> userId = GetFirstGamePlayer()->GetPreferredUniqueNetId().GetUniqueNetId();
        if (TryJoinSession(userId, GameSessionName, mpSessionSearch->SearchResults[foundSessionIndex]))
        {
            return true;
        }
    }
    return false;
}

bool UGameInstance_Unknown::ShutDownCurrentSession(const FEventShutDownCurrentSessionCompleted& onShutDownCurrentSessionCompleted)
{
    mEventShutDownCurrentSessionCompleted = onShutDownCurrentSessionCompleted;
    return TryDestroySession(GameSessionName);
}

bool UGameInstance_Unknown::TryHostSession(TSharedPtr<const FUniqueNetId> userId, FName sessionName, bool isLAN, bool isPresence, int32 maxPlayersNum, const FString& serverName)
{
    IOnlineSubsystem* pOnlineSub = IOnlineSubsystem::Get();
    if (pOnlineSub)
    {
        IOnlineSessionPtr pSession = pOnlineSub->GetSessionInterface();

        if (pSession.IsValid() && userId.IsValid())
        {
            mpSessionSettings = MakeShareable(new FOnlineSessionSettings());

            mpSessionSettings->bIsLANMatch = isLAN;
            mpSessionSettings->bUsesPresence = isPresence;
            mpSessionSettings->NumPublicConnections = maxPlayersNum;
            mpSessionSettings->NumPrivateConnections = 0;
            mpSessionSettings->bAllowInvites = true;
            mpSessionSettings->bAllowJoinInProgress = true;
            mpSessionSettings->bShouldAdvertise = true;
            mpSessionSettings->bAllowJoinViaPresence = true;
            mpSessionSettings->bAllowJoinViaPresenceFriendsOnly = false;

            mpSessionSettings->Set(SETTING_MAPNAME, FString("NO MAP"), EOnlineDataAdvertisementType::ViaOnlineService);
            mpSessionSettings->Set(SETTING_CUSTOMSEARCHINT1, serverName, EOnlineDataAdvertisementType::ViaOnlineService);

            mEventCreateSessionCompletedHandle = pSession->AddOnCreateSessionCompleteDelegate_Handle(mEventCreateSessionCompleted);
            return pSession->CreateSession(*userId, sessionName, *mpSessionSettings);
        }
    }
    return false;
}

bool UGameInstance_Unknown::TryFindSessions(TSharedPtr<const FUniqueNetId> userId, bool isLAN, bool isPresence, int32 maxSearchResultsNum)
{
    IOnlineSubsystem* onlineSub = IOnlineSubsystem::Get();
    if (onlineSub)
    {
        IOnlineSessionPtr pSession = onlineSub->GetSessionInterface();
        if (pSession.IsValid() && userId.IsValid())
        {
            mpSessionSearch = MakeShareable(new FOnlineSessionSearch());
            mpSessionSearch->bIsLanQuery = isLAN;
            mpSessionSearch->MaxSearchResults = maxSearchResultsNum;
            mpSessionSearch->PingBucketSize = 50;
            if (isPresence)
            {
                mpSessionSearch->QuerySettings.Set(SEARCH_PRESENCE, isPresence, EOnlineComparisonOp::Equals);
            }
            mpSessionSearch->TimeoutInSeconds = 5;

            mEventFindSessionsCompletedHandle = pSession->AddOnFindSessionsCompleteDelegate_Handle(mEventFindSessionsCompleted);

            return pSession->FindSessions(*userId, mpSessionSearch.ToSharedRef());
        }
    }
    return false;
}

bool UGameInstance_Unknown::TryJoinSession(TSharedPtr<const FUniqueNetId> userId, FName sessionName, const FOnlineSessionSearchResult& searchResult)
{
    mFoundOnlineSessions.Empty();
    IOnlineSubsystem* pOnlineSub = IOnlineSubsystem::Get();
    if (pOnlineSub)
    {
        IOnlineSessionPtr pSession = pOnlineSub->GetSessionInterface();
        if (pSession.IsValid() && userId.IsValid())
        {
            mEventJoinSessionCompletedHandle = pSession->AddOnJoinSessionCompleteDelegate_Handle(mEventJoinSessionCompleted);
            return pSession->JoinSession(*userId, sessionName, searchResult);
        }
    }
    return false;
}

bool UGameInstance_Unknown::TryDestroySession(FName sessionName)
{
    IOnlineSubsystem* pOnlineSub = IOnlineSubsystem::Get();
    if (pOnlineSub)
    {
        IOnlineSessionPtr pSession = pOnlineSub->GetSessionInterface();
        if (pSession.IsValid())
        {
            mEventDestroySessionCompletedHandle = pSession->AddOnDestroySessionCompleteDelegate_Handle(mEventDestroySessionCompleted);
            pSession->DestroySession(sessionName, mEventDestroySessionCompleted);
            return true;
        }
    }
    return false;
}

void UGameInstance_Unknown::OnCreateSessionCompleted(FName sessionName, bool isSuccessful)
{
    IOnlineSubsystem* pOnlineSub = IOnlineSubsystem::Get();
    if (pOnlineSub)
    {
        IOnlineSessionPtr pSession = pOnlineSub->GetSessionInterface();
        if (pSession.IsValid())
        {
            pSession->ClearOnCreateSessionCompleteDelegate_Handle(mEventCreateSessionCompletedHandle);
            if (isSuccessful)
            {
                mEventStartSessionCompletedHandle = pSession->AddOnStartSessionCompleteDelegate_Handle(mEventStartSessionCompleted);
                pSession->StartSession(sessionName);
                return;
            }
        }
    }
    mEventStartHostSessionCompleted.ExecuteIfBound(false);
}

void UGameInstance_Unknown::OnStartSessionCompleted(FName sessionName, bool isSuccessful)
{
    IOnlineSubsystem* pOnlineSub = IOnlineSubsystem::Get();
    if (pOnlineSub)
    {
        IOnlineSessionPtr pSession = pOnlineSub->GetSessionInterface();
        if (pSession.IsValid())
        {
            pSession->ClearOnStartSessionCompleteDelegate_Handle(mEventStartSessionCompletedHandle);
        }
    }
    mEventStartHostSessionCompleted.ExecuteIfBound(isSuccessful);
}

void UGameInstance_Unknown::OnFindSessionsCompleted(bool isSuccessful)
{
    mFoundOnlineSessions.Empty();
    IOnlineSubsystem* pOnlineSub = IOnlineSubsystem::Get();
    if (pOnlineSub)
    {
        IOnlineSessionPtr pSession = pOnlineSub->GetSessionInterface();
        if (pSession.IsValid())
        {
            pSession->ClearOnFindSessionsCompleteDelegate_Handle(mEventFindSessionsCompletedHandle);

            mFoundOnlineSessions.Reserve(mpSessionSearch->SearchResults.Num());
            for (int32 i = 0; i < mpSessionSearch->SearchResults.Num(); ++i)
            {
                FOnlineSessionSearchResult& searchResult = mpSessionSearch->SearchResults[i];
                FUnknownOnlineSessionInfo foundSession;
                foundSession.mHostID = searchResult.Session.OwningUserId->ToString();
                foundSession.mSessionName = searchResult.Session.GetSessionIdStr();
                foundSession.mHostName = searchResult.Session.OwningUserName;
                searchResult.Session.SessionSettings.Get(SETTING_MAPNAME, foundSession.mMapName);
                //searchResult.Session.SessionSettings.Get(SETTING_GAMEMODE, foundSession.mGameMode);
                foundSession.mPingInMs = searchResult.PingInMs;
                foundSession.mMaxPlayersNum = searchResult.Session.SessionSettings.NumPublicConnections;
                foundSession.mCurrentPlayersNum = foundSession.mMaxPlayersNum - searchResult.Session.NumOpenPublicConnections;
                searchResult.Session.SessionSettings.Get(SETTING_CUSTOMSEARCHINT1, foundSession.mServerName);
                mFoundOnlineSessions.Push(foundSession);
            }
        }
    }
    mEventStartSessionsSearchCompleted.ExecuteIfBound();
}

void UGameInstance_Unknown::OnJoinSessionCompleted(FName sessionName, EOnJoinSessionCompleteResult::Type joinResult)
{
    if (joinResult == EOnJoinSessionCompleteResult::Success)
    {
        IOnlineSubsystem* pOnlineSub = IOnlineSubsystem::Get();
        if (pOnlineSub)
        {
            IOnlineSessionPtr pSession = pOnlineSub->GetSessionInterface();
            if (pSession.IsValid())
            {
                pSession->ClearOnJoinSessionCompleteDelegate_Handle(mEventJoinSessionCompletedHandle);
                APlayerController* pLocalPlayerController = GetFirstLocalPlayerController();
                FString travelURL;
                if (pLocalPlayerController != nullptr &&
                    pSession->GetResolvedConnectString(sessionName, travelURL))
                {
                    pLocalPlayerController->ClientTravel(travelURL, ETravelType::TRAVEL_Absolute);
                    return;
                }
            }
        }
    }
    else
    {
#define LOCTEXT_NAMESPACE "ERROR_Network"
        FText reasonMessage;
        switch (joinResult)
        {
        case EOnJoinSessionCompleteResult::SessionDoesNotExist:
            reasonMessage = LOCTEXT("ERROR_JoinSession_DoesNotExist", "Session doesn't exist.");
            break;

        case EOnJoinSessionCompleteResult::AlreadyInSession:
            reasonMessage = LOCTEXT("ERROR_JoinSession_AlreadyInSession", "Already in session.");
            break;

        case EOnJoinSessionCompleteResult::CouldNotRetrieveAddress:
            reasonMessage = LOCTEXT("ERROR_JoinSession_CouldNotRetrieveAddress", "Could not retrieve address.");
            break;

        case EOnJoinSessionCompleteResult::SessionIsFull:
            reasonMessage = LOCTEXT("ERROR_JoinSession_SessionIsFull", "Session is full.");
            break;

        default:
            reasonMessage = LOCTEXT("ERROR_JoinSession_UnknownError", "Something went wrong...");
        }
#undef LOCTEXT_NAMESPACE
        mEventStartJoinSessionCompleted.ExecuteIfBound(reasonMessage);
    }
}

void UGameInstance_Unknown::OnDestroySessionCompleted(FName sessionName, bool isSuccessful)
{
    IOnlineSubsystem* pOnlineSub = IOnlineSubsystem::Get();
    if (pOnlineSub)
    {
        IOnlineSessionPtr pSession = pOnlineSub->GetSessionInterface();
        if (pSession.IsValid())
        {
            pSession->ClearOnDestroySessionCompleteDelegate_Handle(mEventDestroySessionCompletedHandle);
            //UGameplayStatics::OpenLevel(GetWorld(), mMainMenuLevel, true);
        }
    }
    mEventShutDownCurrentSessionCompleted.ExecuteIfBound(isSuccessful);
}

void UGameInstance_Unknown::OnArenaLobbySessionHosted(bool isSuccessful)
{
    if (isSuccessful)
    {
        UGameplayStatics::OpenLevel(GetWorld(), mArenaLobbyLevel, true, "listen");
    }
}

void UGameInstance_Unknown::OnShutDownCurrentSession(bool isSuccessful)
{
    if (isSuccessful)
    {
        UGameplayStatics::OpenLevel(GetWorld(), mMainMenuLevel, true);
    }
}

void UGameInstance_Unknown::OnNetworkFailure(UWorld* pWorld, UNetDriver* pNetDriver, ENetworkFailure::Type failureType, const FString& errorString)
{
    ShutDownCurrentSession({});
}
