

#pragma once


#include "Engine/DataTable.h"
#include "ProgressManager.h"
#include "DialogueSystem.generated.h"


UENUM(BlueprintType)
enum class ECampTravel : uint8
{
    CampTravel_None,
    CampTravel_Neighbour,
    CampTravel_Unconsciously,
    CampTravel_Direct,
    CampTravel_PathFind,
};


USTRUCT(BlueprintType)
struct FActionScriptData
{
    GENERATED_BODY()

public:

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unknown|GlobalMap")
    FName ActionScriptID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unknown|GlobalMap")
    FName ScenarioID;
};


USTRUCT(BlueprintType)
struct FActionVariant
{
    GENERATED_BODY()

public:

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unknown|GlobalMap")
    FText Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unknown|GlobalMap")
    TArray<FProgressVariableCheck> Conditions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unknown|GlobalMap")
    TArray<FProgressVariableChange> Results;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unknown|GlobalMap")
    FName NextActionBlockID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unknown|GlobalMap")
    TArray<FActionScriptData> ActionScripts;
};


USTRUCT(BlueprintType)
struct FActionBlock
{
    GENERATED_BODY()

public:

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unknown|GlobalMap")
    FName ID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unknown|GlobalMap")
    UTexture2D* HeadPicture;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unknown|GlobalMap")
    FText Title;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unknown|GlobalMap")
    FText Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unknown|GlobalMap")
    TArray<FActionVariant> Variants;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unknown|GlobalMap")
    bool CanLeaveCamp;
};


USTRUCT(BlueprintType)
struct FFirstTurnDialogue
{
    GENERATED_BODY()

public:

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unknown|GlobalMap")
    FName ActionBlockID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unknown|GlobalMap")
    TArray<FProgressVariableCheck> Conditions;
};


USTRUCT()
struct FCampData
    : public FTableRowBase
{
    GENERATED_BODY()

public:

    UPROPERTY(EditAnywhere)
    TArray<FActionBlock> ActionBlocks;

    UPROPERTY(EditAnywhere)
    TArray<FFirstTurnDialogue> FirstTurnDialogues;
};