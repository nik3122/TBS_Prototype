

#include "Arena/Action_Summon_Base.h"
#include "Arena/ArenaUtilities.h"
#include "Arena/BattleLog/BattleLog.h"
#include "Arena/BattleLog/BattleLogItem_ActionSummon.h"
#include "Arena/OccupationSlot.h"
#include "Arena/SocialComponent.h"
#include "Arena/Party.h"
#include "BitmaskOperator.h"


AAction_Summon_Base::AAction_Summon_Base(const FObjectInitializer& objectInitializer)
    : Super(objectInitializer)
{
    //  ~!~ Currently it is forbidden to spawn a creature at the same tile where a trap is spawned.
    mOccupationSlotsToBeFree = MakeBitMask(EOccupationSlot::OccupationSlot_Static, EOccupationSlot::OccupationSlot_ArenaUnit, EOccupationSlot::OccupationSlot_Trap);
    mActionBehavior = MakeBitMask(EActionBehavior::ActionBehavior_Summon);
    mSingleActionAnimationSection = "Summon";
}

void AAction_Summon_Base::PreExecutionApply_Implementation()
{
    Super::PreExecutionApply_Implementation();

    if (HasAuthority())
    {
        USocialComponent* pSocial = IArenaUnit_Interface::Execute_GetSocialComponent(GetExecutorActor());
        check(IsValid(pSocial) && "Arena unit without soical component shouldn't own AAction_Summon_Base");
        TArray<UBattleLogItem_Base*> battleLogItems;
        UBattleLog* pBattleLog = UArenaUtilities::GetBattleLog(this);
        for (AActor* pSpawnedActor : mSpawnedActors)
        {
            auto pSpawnedCreature = Cast<ACreatureBase>(pSpawnedActor);
            checkSlow(IsValid(pSpawnedCreature) && "Spawned actor must inherit ACreatureBase");
            pSocial->GetParty()->AddMember(pSpawnedCreature);

            battleLogItems.Push(pBattleLog->CreateBattleLogItem_ActionSummon(this, pSpawnedCreature));
        }
        if (battleLogItems.Num() > 0)
        {
            pBattleLog->PushBattleLogItems(battleLogItems);
        }
    }
}

void AAction_Summon_Base::OnConstruction(const FTransform& transform)
{
    Super::OnConstruction(transform);

    mActorToSpawnClass = mCreatureToSpawnClass;
}
