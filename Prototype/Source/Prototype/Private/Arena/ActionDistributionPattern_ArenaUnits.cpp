

#include "Arena/ActionDistributionPattern_ArenaUnits.h"
#include "Arena/Action_Base.h"
#include "Arena/TiledMovement.h"
#include "Arena/PassMap.h"
#include "Arena/TiledMovement.h"
#include "Arena/SocialComponent.h"
#include "Arena/CreatureBase.h"
#include "Arena/Party.h"
#include "BitmaskOperator.h"


void UActionDistributionPattern_ArenaUnits::GetSetupTilesPattern(const FIntPoint& executorPlacement,
                                                                 TArray<FIntPoint>& outAvailableSetupTiles,
                                                                 const AAction_Base* pAction) const
{
    /*int32 minRange = pAction->GetRangeMin();
    int32 maxRange = pAction->GetRangeMax();
    APassMap* pPassMap = pAction->GetExecutorTiledMovement()->GetPassMap();
    pPassMap->GetHexaRing(executorPlacement, minRange, maxRange, outAvailableSetupTiles);*/

    auto executorArenaUnit = pAction->GetExecutorArenaUnit();
    USocialComponent* pExecutorSocialComponent = executorArenaUnit->Execute_GetSocialComponent(executorArenaUnit.GetObject());
    AParty* pAllyParty = pExecutorSocialComponent->GetParty();
    AParty* pEnemyParty = pExecutorSocialComponent->GetEnemyParty();
    //  ~?~SLOW~ May be slow if not only arena units will be registered in the PassMap.
    APassMap* pPassMap = pAction->GetExecutorTiledMovement()->GetPassMap();
    for (UTiledMovement* pPassMapMovement : pPassMap->GetRegisteredPassMapMovements())
    {
        AActor* pOccupant = pPassMapMovement->GetOwner();
        FIntPoint occupantTile = pPassMapMovement->GetCurrentTile();
        if (mActionRangeAffectsDistribution && pAction->GetRangeMin() >= 0 && pAction->GetRangeMax() >= 0)
        {
            int32 distanceToTile = pPassMap->GetDistance(executorPlacement, occupantTile);
            if (distanceToTile < pAction->GetRangeMin() || distanceToTile <= pAction->GetRangeMax())
            {
                continue;
            }
        }
        bool shouldIncludeOccupantTile = false;
        if (mIncludeCreaturesOnly)
        {
            ACreatureBase* pOccupantCreature = Cast<ACreatureBase>(pOccupant);
            if (pOccupantCreature != nullptr)
            {
                shouldIncludeOccupantTile = true;
                int32 memberIndex;
                bool isOccupantAlly = pAllyParty->IsInParty(pOccupantCreature, memberIndex);
                bool isOccupantEnemy = pEnemyParty->IsInParty(pOccupantCreature, memberIndex);
                shouldIncludeOccupantTile = !((mIgnoreAllies && isOccupantAlly) ||
                    (mIgnoreEnemies && isOccupantEnemy));
            }
        }
        else
        {
            shouldIncludeOccupantTile = pOccupant->Implements<UArenaUnit_Interface>();
        };
        if (shouldIncludeOccupantTile)
        {
            outAvailableSetupTiles.Push(occupantTile);
        }
    }
}

void UActionDistributionPattern_ArenaUnits::GetDistributionTilesPattern(const FIntPoint& executorPlacement,
                                                                        const FIntPoint& actionPlacement,
                                                                        TArray<FIntPoint>& outDistributionTiles,
                                                                        const AAction_Base* pAction) const
{
    auto executorArenaUnit = pAction->GetExecutorArenaUnit();
    USocialComponent* pExecutorSocialComponent = executorArenaUnit->Execute_GetSocialComponent(executorArenaUnit.GetObject());
    AParty* pAllyParty = pExecutorSocialComponent->GetParty();
    AParty* pEnemyParty = pExecutorSocialComponent->GetEnemyParty();
    //  ~?~SLOW~ May be slow if not only arena units will be registered in the PassMap.
    APassMap* pPassMap = pAction->GetExecutorTiledMovement()->GetPassMap();
    for (UTiledMovement* pPassMapMovement : pPassMap->GetRegisteredPassMapMovements())
    {
        AActor* pOccupant = pPassMapMovement->GetOwner();
        FIntPoint occupantTile = pPassMapMovement->GetCurrentTile();
        if (mActionRangeAffectsDistribution && pAction->GetRangeMin() >= 0 && pAction->GetRangeMax() >= 0)
        {
            int32 distanceToTile = pPassMap->GetDistance(executorPlacement, occupantTile);
            if (distanceToTile < pAction->GetRangeMin() || distanceToTile <= pAction->GetRangeMax())
            {
                continue;
            }
        }
        bool shouldIncludeOccupantTile = false;
        if (mIncludeCreaturesOnly)
        {
            ACreatureBase* pOccupantCreature = Cast<ACreatureBase>(pOccupant);
            if (pOccupantCreature != nullptr)
            {
                shouldIncludeOccupantTile = true;
                int32 memberIndex;
                bool isOccupantAlly = pAllyParty->IsInParty(pOccupantCreature, memberIndex);
                bool isOccupantEnemy = pEnemyParty->IsInParty(pOccupantCreature, memberIndex);
                shouldIncludeOccupantTile = !((mIgnoreAllies && isOccupantAlly) ||
                    (mIgnoreEnemies && isOccupantEnemy));
            }
        }
        else
        {
            shouldIncludeOccupantTile = pOccupant->Implements<UArenaUnit_Interface>();
        };
        if (shouldIncludeOccupantTile)
        {
            outDistributionTiles.Push(occupantTile);
        }
    }
}
