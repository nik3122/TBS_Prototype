#pragma once


#include "ActionScript_Base.h"
#include "ProgressManager.h"
#include "ActionScript_LoadArena.generated.h"


class AParty;
class ACreatureBase;


USTRUCT()
struct FArenaResultScenario
{
    GENERATED_BODY()

public:

    UPROPERTY(EditAnywhere)
    TArray<FProgressVariableChange> ProgressOnWin;

    UPROPERTY(EditAnywhere)
    TArray<FProgressVariableChange> ProgressOnLoose;

    UPROPERTY(EditAnywhere)
    TArray<FProgressVariableChange> ProgressOnDraw;
};


USTRUCT()
struct FLoadArenaScenario
    : public FTableRowBase
{
    GENERATED_BODY()

public:

    UPROPERTY(EditAnywhere)
    FName ArenaLevelName;

    UPROPERTY(EditAnywhere)
    TArray<FName> PlayerPartyMembers;

    UPROPERTY(EditAnywhere)
    bool DontUseGMPlayerParty;

    UPROPERTY(EditAnywhere)
    TArray<FName> AIPartyMembers;

    UPROPERTY(EditAnywhere)
    FArenaResultScenario ArenaResultScenario;
};


UCLASS()
class AActionScript_LoadArena
    : public AActionScript_Base
{
    GENERATED_BODY()

public:

    AActionScript_LoadArena();

protected:

    void ExecuteInner_Implementation(const FName& scenarioID) override;

    void FillPartyWithMembers(const TArray<FName>& memberNames, TArray<TSubclassOf<ACreatureBase>>& partyMembers);
};