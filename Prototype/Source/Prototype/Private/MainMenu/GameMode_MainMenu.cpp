

#include "MainMenu/GameMode_MainMenu.h"


AGameMode_MainMenu::AGameMode_MainMenu(const FObjectInitializer& objectInitializer)
    : Super(objectInitializer)
{
    
}

void AGameMode_MainMenu::PreLogin(const FString& options,
                                  const FString& address,
                                  const FUniqueNetIdRepl& uniqueId,
                                  FString& errorMessage)
{
    Super::PreLogin(options, address, uniqueId, errorMessage);

    GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Blue, TEXT("PreLogin"));
}

void AGameMode_MainMenu::PostLogin(APlayerController* pNewPlayer)
{
    Super::PostLogin(pNewPlayer);

    GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Blue, TEXT("PostLogin"));
}
