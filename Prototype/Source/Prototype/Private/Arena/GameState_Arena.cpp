

#include "Arena/GameState_Arena.h"
#include "Arena/ArenaUtilities.h"
#include "Arena/ArenaSetupInfo.h"
#include "Arena/ArenaManager.h"
#include "Arena/TurnsManager.h"
#include "Arena/Party.h"
#include "Arena/PlayerController_ArenaFight.h"
#include "EngineUtils.h"
#include "GameInstance_Unknown.h"
#include "Net/UnrealNetwork.h"
#include "Kismet/GameplayStatics.h"
#include "UnknownCommon.h"

#include "Arena//ArenaStateSerDes.h"


void AGameState_Arena::EndOfArena(const FArenaResults& arenaResults)
{
    Multicast_EndOfArena(arenaResults);
}

bool AGameState_Arena::HasMatchStarted() const
{
    return Super::HasMatchStarted() && mIsBattleStarted;
}

void AGameState_Arena::BeginPlay()
{
    Super::BeginPlay();

    mpArenaManager = *TActorIterator<AArenaManager>(GetWorld());
    check(IsValid(mpArenaManager) && "AGameState_Arena can't detect any AArenaManager instance.");

    FOnSystemsSetupDelegate onSystemsSetup;
    onSystemsSetup.BindDynamic(this, &AGameState_Arena::OnSystemsSetup);
    UArenaUtilities::ExecuteWhenSystemsSetup(this, onSystemsSetup);
}

void AGameState_Arena::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(AGameState_Arena, mArenaGameMode);
    DOREPLIFETIME(AGameState_Arena, mpPlayerStateOne);
    DOREPLIFETIME(AGameState_Arena, mpPlayerStateTwo);
}

AArenaManager* AGameState_Arena::GetArenaManager() const
{
    return mpArenaManager;
}

EArenaGameMode AGameState_Arena::GetArenaGameMode() const
{
    return mArenaGameMode;
}

APlayerState_Arena* AGameState_Arena::GetMyPlayerState() const
{
     auto pPlayerController = Cast<APlayerController_ArenaFight>(UGameplayStatics::GetPlayerController(this, 0));
     return pPlayerController != nullptr ? pPlayerController->GetArenaPlayerState() : nullptr;
}

APlayerState_Arena* AGameState_Arena::GetOpponentPlayerState() const
{
    return GetMyPlayerState() == mpPlayerStateOne ? mpPlayerStateTwo : mpPlayerStateOne;
}

void AGameState_Arena::SaveBugReport(const FBugReportInfo& bugReportInfo)
{
    if (mpArenaStateTurnInitial == nullptr ||
        mpArenaStateTurnFinal == nullptr)
    {
        return;
    }

    //  Flush arena state to the file:
    int32 currentTurn = UArenaUtilities::GetTurnsManager(this)->GetCurrentTurn();
    FString dateTime = FDateTime::Now().ToString(TEXT("%d.%m.%y_%H-%M-%S"));
    FString fileName = FString::Printf(TEXT("bugreport_%i-%i_[%s].txt"),
                                       currentTurn,
                                       currentTurn + 1,
                                       *dateTime);
    FString filePath = FPaths::ProjectDir() + "\\Reports\\" + fileName;

    FString reportText;
    {
        tinyxml2::XMLPrinter printer;

        printer.OpenElement("capture");
        printer.PushText(TCHAR_TO_ANSI(*bugReportInfo.mReportName));
        printer.CloseElement();

        printer.OpenElement("custominfo");
        printer.PushText(TCHAR_TO_ANSI(*bugReportInfo.mCustomInfo));
        printer.CloseElement();

        printer.OpenElement("initial");
        StateSerDes<State_Arena>::SerializeXML(*mpArenaStateTurnInitial, printer);
        printer.CloseElement();

        printer.OpenElement("resolved");
        StateSerDes<State_Arena>::SerializeXML(*mpArenaStateTurnResolved, printer);
        printer.CloseElement();

        printer.OpenElement("final");
        StateSerDes<State_Arena>::SerializeXML(*mpArenaStateTurnFinal, printer);
        printer.CloseElement();

        reportText = ANSI_TO_TCHAR(printer.CStr());
    }

    //IPlatformFile& platformFile = FPlatformFileManager::Get().GetPlatformFile();

    FFileHelper::SaveStringToFile(
        reportText,
        *filePath
    );
}

void AGameState_Arena::StoreState_Inner(ArenaStateContext& context, State_Arena& state) const
{
    state.mTurn = UArenaUtilities::GetTurnsManager(this)->GetCurrentTurn();

    AArenaManager* pArenaManager = UArenaUtilities::GetArenaManager(this);
    if (pArenaManager != nullptr)
    {
        //  Save state for Player party:
        AParty* pPartyOne = pArenaManager->GetPartyOne_Implementation();
        if (pPartyOne != nullptr)
        {
            const TArray<ACreatureBase*>& partyMembers = pPartyOne->GetMembers();
            state.mPlayerPartyMembersStatesCount = partyMembers.Num();
            if (state.mPlayerPartyMembersStatesCount > 0)
            {
                state.mppPlayerPartyMembersStates = context.AllocMemory<State_CreatureBase*>(state.mPlayerPartyMembersStatesCount);
                for (int32 i = 0; i < partyMembers.Num(); ++i)
                {
                    state.mppPlayerPartyMembersStates[i] = partyMembers[i]->StoreState(context);
                }
            }
        }
        else
        {
            state.mPlayerPartyMembersStatesCount = 0;
            state.mppPlayerPartyMembersStates = nullptr;
        }

        //  Save state for AI party:
        AParty* pPartyTwo = pArenaManager->GetPartyTwo_Implementation();
        if (pPartyTwo != nullptr)
        {
            const TArray<ACreatureBase*>& partyMembers = pPartyTwo->GetMembers();
            state.mAIPartyMembersStatesCount = partyMembers.Num();
            if (state.mAIPartyMembersStatesCount > 0)
            {
                state.mppAIPartyMembersStates = context.AllocMemory<State_CreatureBase*>(state.mAIPartyMembersStatesCount);
                for (int32 i = 0; i < partyMembers.Num(); ++i)
                {
                    state.mppAIPartyMembersStates[i] = partyMembers[i]->StoreState(context);
                }
            }
        }
        else
        {
            state.mAIPartyMembersStatesCount = 0;
            state.mppAIPartyMembersStates = nullptr;
        }
    }

    state.mpPassMapState = pArenaManager->GetPassMap_Implementation()->StoreState(context);

    state.mpFireSystemState = pArenaManager->GetFireSystem_Implementation()->StoreState(context);
}

bool AGameState_Arena::RestoreState_Inner(const State_Arena& state)
{
    //  ~TODO~
    return false;
}

void AGameState_Arena::Multicast_EndOfArena_Implementation(FArenaResults arenaResults)
{
    mIsBattleFinished = true;
    mArenaResults = arenaResults;
    mEventEndOfArena.Broadcast();
}

void AGameState_Arena::OnSystemsSetup()
{
    ATurnsManager* pTurnsManager = UArenaUtilities::GetTurnsManager(this);
    pTurnsManager->mEventStoreTurnStateInitial.BindUObject(this, &AGameState_Arena::OnStoreTurnStateInitial);
    pTurnsManager->mEventStoreTurnStateResolved.BindUObject(this, &AGameState_Arena::OnStoreTurnStateResolved);
    pTurnsManager->mEventStoreTurnStateFinal.BindUObject(this, &AGameState_Arena::OnStoreTurnStateFinal);
}

void AGameState_Arena::OnStoreTurnStateInitial()
{
    mArenaStateContextTurnInitial.Reset();
    mpArenaStateTurnInitial = StoreState(mArenaStateContextTurnInitial);
    UE_LOG(ArenaCommonLog, Verbose, TEXT("Turn state stored: initial."));
}

void AGameState_Arena::OnStoreTurnStateResolved()
{
    mArenaStateContextTurnResolved.Reset();
    mpArenaStateTurnResolved = StoreState(mArenaStateContextTurnResolved);
    UE_LOG(ArenaCommonLog, Verbose, TEXT("Turn state stored: resolved."));
}

void AGameState_Arena::OnStoreTurnStateFinal()
{
    mArenaStateContextTurnFinal.Reset();
    mpArenaStateTurnFinal = StoreState(mArenaStateContextTurnFinal);
    UE_LOG(ArenaCommonLog, Verbose, TEXT("Turn state stored: final."));
    SaveTurnStateToFile();
}

void AGameState_Arena::SaveTurnStateToFile()
{
    //return;
    //  Flush arena state to the file:
    int32 currentTurn = UArenaUtilities::GetTurnsManager(this)->GetCurrentTurn();
    FString fileName = TEXT("autostate.txt");
    FString filePath = FPaths::ProjectDir() + "\\Reports\\" + fileName;

    FString xmlResult;
    {
        tinyxml2::XMLPrinter printer;

        printer.OpenElement("initial");
        StateSerDes<State_Arena>::SerializeXML(*mpArenaStateTurnInitial, printer);
        printer.CloseElement();

        printer.OpenElement("resolved");
        StateSerDes<State_Arena>::SerializeXML(*mpArenaStateTurnResolved, printer);
        printer.CloseElement();

        printer.OpenElement("final");
        StateSerDes<State_Arena>::SerializeXML(*mpArenaStateTurnFinal, printer);
        printer.CloseElement();

        xmlResult = ANSI_TO_TCHAR(printer.CStr());
    }

    //IPlatformFile& platformFile = FPlatformFileManager::Get().GetPlatformFile();

    FFileHelper::SaveStringToFile(
        xmlResult,
        *filePath
    );
}
