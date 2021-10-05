#pragma once


#include "CoreMinimal.h"
#include "GameFramework/GameSession.h"
#include "GameSession_ArenaLobby.generated.h"


UCLASS()
class AGameSession_ArenaLobby
    : public AGameSession
{
    GENERATED_BODY()

public:

    AGameSession_ArenaLobby(const FObjectInitializer& objectInitializer);
};