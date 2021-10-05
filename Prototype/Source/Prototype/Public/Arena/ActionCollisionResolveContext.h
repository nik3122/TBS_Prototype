#pragma once


#include "ActionCollisionResolveContext.generated.h"


class AActionMap;
class AAction_Base;


//  Presents the "on-going" state of the actions' collisions resolving process.
//  Used for movement collision resolve.
USTRUCT(BlueprintType)
struct FActionCollisionResolveContext
{
    GENERATED_BODY()

public:

    //  Movement collision resolve:

    bool IsTilePretended(const FIntPoint& tile) const;

    bool SetTilePretender(const FIntPoint& tile, AAction_Base* pNewPretender);

    void FreeTilePretender(const FIntPoint& tile, AAction_Base* pOldPretender);


private:

    TMap<FIntPoint, AAction_Base*> mNextTurnTilesPretenders;

};