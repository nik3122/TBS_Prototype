

#include "Arena/ActionCollisionResolveContext.h"
#include "Arena/ArenaUtilities.h"
#include "Arena/Action_Base.h"



bool FActionCollisionResolveContext::IsTilePretended(const FIntPoint& tile) const
{
    return mNextTurnTilesPretenders.Find(tile) != nullptr;
}

bool FActionCollisionResolveContext::SetTilePretender(const FIntPoint& tile, AAction_Base* pNewPretender)
{
    AAction_Base*& pPretender = mNextTurnTilesPretenders.FindOrAdd(tile);
    if (pPretender == nullptr)
    {
        pPretender = pNewPretender;
        return true;
    }
    else
    {
        UE_LOG(ArenaActionsLog, Warning, TEXT("Tile [%i, %i] is already pretended by %s"), tile.X, tile.Y, *pPretender->GetName());
    }
    return false;
}

void FActionCollisionResolveContext::FreeTilePretender(const FIntPoint& tile, AAction_Base* pOldPretender)
{
    if (mNextTurnTilesPretenders.Remove(tile) == 0)
    {
        UE_LOG(ArenaActionsLog, Warning, TEXT("No tile pretendent (%s) was found for tile [%i, %i]"), *pOldPretender->GetName(), tile.X, tile.Y);
    }
}
