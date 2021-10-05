

#include "Arena/Actions/Action_SeparateMovement_Range.h"
#include "Arena/ActionManager.h"
#include "Arena/ArenaUnit_Interface.h"
#include "Arena/Damageable.h"
#include "Arena/PassMap.h"
#include "Arena/TiledMovement.h"


AAction_SeparateMovement_Range::AAction_SeparateMovement_Range(const FObjectInitializer& initializer)
    : Super(initializer)
{
    mIgnoreHighAndLowObstacle = false;
    mIgnoreLowObstacle = true;
}

void AAction_SeparateMovement_Range::AffectVictims()
{
    mVictims.SetNum(1, false);

    /*
    FVictimInfo* primaryVictim = nullptr;
    int32 primaryVictimTileDistance = 9999;
    int32 primaryVictimExecutionPriority = 0;

    APassMap* passMap = GetExecutorTiledMovement()->GetPassMap();
    FIntPoint executorFinalTile = GetFinalTile();

    for (FVictimInfo& victim : mVictims)
    {
        if (victim.mArenaUnit.IsValid())
        {
            int32 victimTileDistance = passMap->GetDistance(executorFinalTile, victim.mTile);
            UActionManager* victimActionManager = IArenaUnit_Interface::Execute_GetActionManager(victim.mArenaUnit.Get());
            int32 victimExecutionPriority = victimActionManager != nullptr ? victimActionManager->GetActionToExecute()->GetExecutionPriority() : 0;

            if (victimTileDistance < primaryVictimTileDistance ||
                (victimTileDistance == primaryVictimTileDistance &&
                 victimExecutionPriority > primaryVictimExecutionPriority))
            {
                primaryVictim = &victim;
                primaryVictimTileDistance = victimTileDistance;
                primaryVictimExecutionPriority = victimExecutionPriority;
            }
        }
    }

    if (primaryVictim != nullptr)
    {
        FVictimInfo victimCopy = *primaryVictim;
        mVictims.SetNum(0, false);
        mVictims.Push(victimCopy);
    }*/

    Super::AffectVictims();
}
