#pragma once


#include "CoreMinimal.h"
#include "Arena/ActionMap.h"
#include "ActionMap_MixedConflicts.generated.h"


UCLASS()
class AActionMap_MixedConflicts
    : public AActionMap
{
    GENERATED_BODY()

public:

protected:

    void SubmitAction_Inner(AAction_Base* action,
                            int32 executionPriority) override;

    void ResolveActionConflicts_Inner() override;

    void OnPrepareForPlanning() override;

private:

    struct ActionEntry
    {
        int32 mPriority;
        int32 mOrder;
        AAction_Base* mpAction;
        int32 mEntryBehavior;
        FIntPoint mTile;

        bool operator==(const ActionEntry& right) const
        {
            return (mPriority == right.mPriority) &&
                (mOrder == right.mOrder) &&
                (mpAction == right.mpAction);
        }
    };

    TArray<ActionEntry> mSubmittedActionsEntries;

    TArray<AAction_Base*> mSubmittedActions;

    int32 mPriorityShiftAboveUnitValue = 0;
};