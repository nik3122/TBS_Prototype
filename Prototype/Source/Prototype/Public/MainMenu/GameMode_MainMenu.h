#pragma once


#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Net/UnrealNetwork.h"
#include "GameMode_MainMenu.generated.h"


UCLASS()
class AGameMode_MainMenu
    : public AGameModeBase
{
    GENERATED_BODY()

public:

    AGameMode_MainMenu(const FObjectInitializer& objectInitializer);

    void PreLogin(const FString& options,
                  const FString& address,
                  const FUniqueNetIdRepl& uniqueId,
                  FString& errorMessage) override;

    void PostLogin(APlayerController* pNewPlayer) override;
};