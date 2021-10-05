#pragma once


#include "GameFramework/SaveGame.h"
#include "GameProgressSaveGame.generated.h"


USTRUCT(BlueprintType)
struct FSaveSlot
{
    GENERATED_BODY()

public:

    UPROPERTY(BlueprintReadWrite, Category = "Unknown")
    FString mSlotName;

    UPROPERTY(BlueprintReadWrite, Category = "Unknown")
    int32 mUserIndex = 0;

    UPROPERTY(BlueprintReadWrite, Category = "Unknown")
    FName mGlobalMapLevelName;

    UPROPERTY(BlueprintReadWrite, Category = "Unknown")
    FDateTime mSaveDate;

    UPROPERTY(BlueprintReadWrite, Category = "Unknown")
    int32 mProgressPercentage;

    UPROPERTY(BlueprintReadWrite, Category = "Unknown")
    FText mCurrentCampName;

public:

    bool operator==(const FSaveSlot& right) const
    {
        return (mSlotName == right.mSlotName) && (mUserIndex == right.mUserIndex);
    }

    bool operator!=(const FSaveSlot& right) const
    {
        return !operator==(right);
    }
};


UCLASS()
class PROTOTYPE_API UGameProgressSaveGame
    : public USaveGame
{
    GENERATED_BODY()

public:

    UPROPERTY(BlueprintReadWrite, Category = "Unknown")
    TArray<FSaveSlot> mSaveSlots;
};