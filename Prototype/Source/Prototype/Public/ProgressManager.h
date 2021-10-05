// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CampaignSaveGame.h"
#include "Engine/DataTable.h"
#include "UObject/NoExportTypes.h"
#include "ProgressManager.generated.h"


UENUM(BlueprintType)
enum class ECompare : uint8
{
    Compare_Less,
    Compare_LessEqual,
    Compare_Equal,
    Compare_MoreEqual,
    Compare_More,
};


USTRUCT(BlueprintType)
struct FProgressVariableCheck
{
    GENERATED_BODY()

public:

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unknown|Progress")
    FName ProgressVariable;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unknown|Progress")
    ECompare Condition;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unknown|Progress")
    int32 Value;
};


UENUM(BlueprintType)
enum class EProgressVariableOperation : uint8
{
    ProgressVariableOperation_Set,
    ProgressVariableOperation_Add,
};


USTRUCT(BlueprintType)
struct FProgressVariableChange
{
    GENERATED_BODY()

public:

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unknown|Progress")
    FName ProgressVariable;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unknown|Progress")
    EProgressVariableOperation Operation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unknown|Progress")
    int32 Value;
};


USTRUCT(BlueprintType)
struct FProgressVariable
    : public FTableRowBase
{
    GENERATED_BODY()

public:

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 Value;
};


UCLASS()
class PROTOTYPE_API UProgressManager : public UObject
{
	GENERATED_BODY()

public:

    DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FEventProgressVariableChanged, FName, progressVariable, int32, oldValue, int32, newValue);
    UPROPERTY(BlueprintAssignable, Category = "Unknown|Progress")
    FEventProgressVariableChanged mEventProgressVariableChanged;
	
public:

    UProgressManager(const FObjectInitializer& objectInitializer = FObjectInitializer::Get());

    void SetProgressVariablesTable(UDataTable* pProgressVariablesTable);

    void SaveState(FProgressManagerState& progressManagerState);

    void LoadState(const FProgressManagerState& progressManagerState);

    void ResetProgress();

    UFUNCTION(BlueprintPure, Category = "Unknown|Progress")
    int32 GetProgressVariableValue(const FName& progressVariable, bool& doesExist);

    UFUNCTION(BlueprintPure, Category = "Unknown|Progress")
    bool CheckProgressVariable(const FProgressVariableCheck& check, bool& doesExist);

    UFUNCTION(BlueprintPure, Category = "Unknown|Progress")
    bool MultiCheckProgressVariable(const TArray<FProgressVariableCheck>& checks);

    UFUNCTION(BlueprintCallable, Category = "Unknown|Progress")
    void ChangeProgressVariable(const FProgressVariableChange& change, bool& doesExist);

    UFUNCTION(BlueprintPure, Category = "Unknown|Progress")
    const TMap<FName, int32>& GetActiveProgressVariables() const;

protected:

    UPROPERTY(BlueprintReadOnly, Category = "Unknown|Progress")
    TMap<FName, int32> mActiveProgressVariables;
	
    UPROPERTY(BlueprintReadOnly, Category = "Unknown|Progress")
    UDataTable* mpProgressVariablesTable;

};
