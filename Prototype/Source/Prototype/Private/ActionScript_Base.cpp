// Fill out your copyright notice in the Description page of Project Settings.

#include "ActionScript_Base.h"
#include "Engine/DataTable.h"
#include "UnknownCommon.h"


void AActionScript_Base::Execute(const FName& scenarioID)
{
    ExecuteInner_Implementation(scenarioID);
}

void AActionScript_Base::FinishExecution(bool executionSucceeded)
{
    mEventExecutionFinished.ExecuteIfBound();
}

void AActionScript_Base::ExecuteInner_Implementation(const FName& parametersID)
{
#if !UE_BUILD_SHIPPING
    UE_LOG(UnknownCommonLog, Warning, TEXT("ExecuteInner is not implemented for %s"), *GetName());
#endif // #if !UE_BUILD_SHIPPING
}

AActionScript_Base::AActionScript_Base()
    : mpScenariosTable(nullptr)
{
}

FName AActionScript_Base::GetActionScriptID() const
{
    return mActionScriptID;
}

void AActionScript_Base::Setup(UDataTable* pScenariosTable)
{
    mpScenariosTable = pScenariosTable;
}
