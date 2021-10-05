// Fill out your copyright notice in the Description page of Project Settings.

#include "ProgressManager.h"


UProgressManager::UProgressManager(const FObjectInitializer& objectInitializer)
    : UObject(objectInitializer)
{}

void UProgressManager::SetProgressVariablesTable(UDataTable* pProgressVariablesTable)
{
    mpProgressVariablesTable = pProgressVariablesTable;
}

void UProgressManager::SaveState(FProgressManagerState& progressManagerState)
{
    progressManagerState.mActiveProgressVariables = mActiveProgressVariables;
}

void UProgressManager::LoadState(const FProgressManagerState& progressManagerState)
{
    mActiveProgressVariables = progressManagerState.mActiveProgressVariables;
}

void UProgressManager::ResetProgress()
{
    mActiveProgressVariables.Empty();
}

int32 UProgressManager::GetProgressVariableValue(const FName& progressVariable, bool& doesExist)
{
    doesExist = true;
    int32* pValue = mActiveProgressVariables.Find(progressVariable);
    if (pValue != nullptr)
    {
        return *pValue;
    }
    
    auto pProgressVariable = mpProgressVariablesTable->FindRow<FProgressVariable>(progressVariable, "");
    if (pProgressVariable != nullptr)
    {
        return pProgressVariable->Value;
    }

    doesExist = false;
    return 0;
}

bool UProgressManager::CheckProgressVariable(const FProgressVariableCheck& check, bool& doesExist)
{
    int32 value = GetProgressVariableValue(check.ProgressVariable, doesExist);
    if (doesExist == false)
    {
        return false;
    }
    switch (check.Condition)
    {
    case ECompare::Compare_Less:
        return value < check.Value;
        
    case ECompare::Compare_LessEqual:
        return value <= check.Value;

    case ECompare::Compare_Equal:
        return value == check.Value;

    case ECompare::Compare_MoreEqual:
        return value >= check.Value;

    case ECompare::Compare_More:
        return value > check.Value;

    default:
        return false;
    }
}

bool UProgressManager::MultiCheckProgressVariable(const TArray<FProgressVariableCheck>& checks)
{
    for (const FProgressVariableCheck& check : checks)
    {
        bool doesExist;
        if (CheckProgressVariable(check, doesExist) == false)
        {
            return false;
        }
    }
    return true;
}

void UProgressManager::ChangeProgressVariable(const FProgressVariableChange& change, bool& doesExist)
{
    int32 oldValue = GetProgressVariableValue(change.ProgressVariable, doesExist);
    if (doesExist == false)
    {
        return;
    }
    int32 newValue = 0;
    switch (change.Operation)
    {
    case EProgressVariableOperation::ProgressVariableOperation_Add:
        newValue = oldValue + change.Value;
        break;

    case EProgressVariableOperation::ProgressVariableOperation_Set:
        newValue = change.Value;
        break;
    }
    mActiveProgressVariables.Add(change.ProgressVariable, newValue);
    mEventProgressVariableChanged.Broadcast(change.ProgressVariable, oldValue, newValue);
}

const TMap<FName, int32>& UProgressManager::GetActiveProgressVariables() const
{
    return mActiveProgressVariables;
}
