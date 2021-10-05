#pragma once


#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "ArenaLobbyMapInfo.generated.h"


USTRUCT(BlueprintType)
struct FArenaLobbyMapInfo
    : public FTableRowBase
{
    GENERATED_BODY()

public:

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ArenaLobby")
    FName mLevelName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ArenaLobby")
    UTexture2D* mpThumbnail = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ArenaLobby")
    FText mLevelNameLoc;
};