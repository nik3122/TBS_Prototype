// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Info.h"
#include "ActionScript_Base.generated.h"


class UDataTable;


UCLASS()
class PROTOTYPE_API AActionScript_Base
    : public AInfo
{
	GENERATED_BODY()
	
public:

    AActionScript_Base();

    FName GetActionScriptID() const;

    DECLARE_DELEGATE(FEventExecutionFinished);
    FEventExecutionFinished mEventExecutionFinished;
	
public:

    void Setup(UDataTable* pScenariosTable);

    UFUNCTION(BlueprintCallable, Category = "Unknown|ActionScript")
    void Execute(const FName& scenarioID);

protected:

    UPROPERTY(EditDefaultsOnly, Category = "Unknown|ActionScript")
    FName mActionScriptID;

    UPROPERTY(BlueprintReadOnly, Category = "Unknown|ActionScript")
    UDataTable* mpScenariosTable;

protected:

    UFUNCTION(BlueprintCallable, Category = "Unknown|ActionScript")
    void FinishExecution(bool executionSucceeded);

    UFUNCTION(BlueprintNativeEvent, Category = "Unknown|ActionScript")
    void ExecuteInner(const FName& scenarioID);
    virtual void ExecuteInner_Implementation(const FName& scenarioID);

private:

    
};
