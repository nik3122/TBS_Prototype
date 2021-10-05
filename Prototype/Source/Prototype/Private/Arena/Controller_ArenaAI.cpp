// Fill out your copyright notice in the Description page of Project Settings.

#include "Arena/Controller_ArenaAI.h"
#include "Arena/ArenaManager.h"
#include "Arena/ArenaUtilities.h"
#include "Arena/TurnsManager.h"
#include "Arena/ArenaManager_Interface.h"
#include "Arena/Party.h"
#include "Arena/Action_Base.h"
#include "Arena/CreatureBase.h"
#include "Arena/Damageable.h"
#include "Arena/ActionManager.h"
#include "Arena/TiledMovement.h"
#include "Arena/Action_MoveTo.h"
#include "Arena/PassMap.h"
#include "BitmaskOperator.h"
#include "UnknownCommon.h"


namespace
{
    struct EnemyPriprity
    {
        ACreatureBase* mpCreature = nullptr;
        float mPriority = 0.0f;
    };
}

AController_ArenaAI::AController_ArenaAI()
{
    //	~!~ ~?~ It can change in future as the spreaded AI decision making can use each tick to
    //	compute the tree of possible decisions.
    PrimaryActorTick.bCanEverTick = false;
}

void AController_ArenaAI::SetAIConfig(const FArenaAIConfig& arenaAIConfig)
{
    mArenaAIConfig = arenaAIConfig;
}

void AController_ArenaAI::AIMakeDecision()
{
    AArenaManager* pArenaManager = UArenaUtilities::GetArenaManager(this);
    AParty* pEnemyParty = IArenaManager_Interface::Execute_GetPlayerParty(pArenaManager);
    if (mpParty->IsNotEmpty() && pEnemyParty->IsNotEmpty())
    {
        //  Sort enemies in order of HP decreasing:
        TArray<ACreatureBase*> sortedEnemies = pEnemyParty->GetMembers();
        sortedEnemies.Sort([](const ACreatureBase& left, const ACreatureBase& right) {
            return left.GetDamageable()->GetCurrentHP() > right.GetDamageable()->GetCurrentHP();
        });

        const TArray<ACreatureBase*>& members = mpParty->GetMembers();
        for (ACreatureBase* pMember : members)
        {
            //  For each damaging action trying to find first enemy for which the action can be applied.
            //  If no possible actions found - moving toward the closes enemy:
            UActionManager* pActionManager = pMember->GetActionManager();
            const TArray<AAction_Base*>& memberActions = pActionManager->GetAllActions();
            bool isAttackActionApplied = false;
            TArray<AAction_Base*> moveActions;
            int32 distanceActionsCount = 0;
            int32 meleeActionsCount = 0;
            for (AAction_Base* pAction : memberActions)
            {
                int32 actionBehavior = pAction->GetBehavior();
                if (actionBehavior == MakeBitMask(EActionBehavior::ActionBehavior_Move))
                {
                    moveActions.Push(pAction);
                }
                else if (IsFlagAtBitMask(actionBehavior, EActionBehavior::ActionBehavior_Damage))
                {
                    for (ACreatureBase* pEnemyCreature : sortedEnemies)
                    {
                        FIntPoint enemyTile = pEnemyCreature->GetTiledMovement()->GetCurrentTile();
                        pAction->SetPlacement(enemyTile);
                        if (pAction->Setup())
                        {
                            pActionManager->SetActionToExecute(pAction);
                            isAttackActionApplied = true;
                            UE_LOG(ArenaAILog, VeryVerbose, TEXT("AI action [%s] set to execute for [%s]."), *pAction->GetName(), *pMember->GetName());
                            break;
                        }
                    }
                }
                if (isAttackActionApplied)
                {
                    break;
                }
            }

            if (isAttackActionApplied == false)
            {
                if (moveActions.Num() > 0)
                {
                    APassMap* pPassMap = UArenaUtilities::GetPassMap(this);
                    UTiledMovement* pMemberPassMapMovement = pMember->GetTiledMovement();
                    TArray<FIntPoint> reachableTiles;
                    pPassMap->GetReachableTiles(pMemberPassMapMovement->GetCurrentTile(),
                                                pMemberPassMapMovement->GetTileSpeed(),
                                                MakeBitMask(EOccupationSlot::OccupationSlot_Static, EOccupationSlot::OccupationSlot_ArenaUnit),
                                                reachableTiles);
                    if (reachableTiles.Num() > 0)
                    {
                        for (AAction_Base* pMoveAction : moveActions)
                        {
                            int32 randomIndex = FMath::RandRange(0, reachableTiles.Num() - 1);
                            pMoveAction->SetPlacement(reachableTiles[randomIndex]);
                            if (pMoveAction->Setup())
                            {
                                pActionManager->SetActionToExecute(pMoveAction);
                                UE_LOG(ArenaAILog, VeryVerbose, TEXT("AI action [%s] set to execute for [%s]."), *pMoveAction->GetName(), *pMember->GetName());
                                break;
                            }
                        }
                    }
                }
            }
        }
    }
}

void AController_ArenaAI::BeginPlay()
{
    Super::BeginPlay();

    UNKNOWN_NOT_ARENA_EDITOR_BEGIN(this)

    UNKNOWN_NOT_ARENA_EDITOR_END
}

void AController_ArenaAI::OnPossess(APawn* pPossessedPawn)
{
    mpParty = Cast<AParty>(pPossessedPawn);
#if !UE_BUILD_SHIPPING
    if (mpParty == nullptr)
    {
        UE_LOG(UnknownCommonLog, Error, TEXT("Trying to possess non-Party pawn by Controller_PartyAI."));
    }
#endif // #if !UE_BUILD_SHIPPING

    Super::OnPossess(pPossessedPawn);
}
