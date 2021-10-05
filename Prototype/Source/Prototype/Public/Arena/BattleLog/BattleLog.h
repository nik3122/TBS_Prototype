#pragma once


#include "CoreMinimal.h"
#include "Arena/BattleLog/BattleLogCommon.h"
#include "Arena/BattleLog/BattleLogItem_Base.h"
#include "Arena/Damage.h"
#include "BattleLog.generated.h"


class AAction_Base;
class ACreatureBase;
class AStatusEffect;
class ATrap_Base;
class UBattleLogItem_Base;
class UBattleLogItem_DamageCreature;
class UBattleLogItem_HealCreature;
class UBattleLogItem_CreatureMovement;
class UBattleLogItem_RamCreature;
class UBattleLogItem_PushCreature;
class UBattleLogItem_CreatureDeath;
struct FStatusEffectTask;


UCLASS()
class UBattleLog
    : public UActorComponent
{
    GENERATED_BODY()

public:

    //  First param - battle log, second param - number of freshly added items.
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FEventBattleLogUpdated, const TArray<UBattleLogItem_Base*>&, battleLogItems, int32, newItemsCount);
    UPROPERTY(BlueprintAssignable, Category = "BattleLog")
    FEventBattleLogUpdated mEventBattleLogUpdated;

public:

    UFUNCTION(BlueprintPure, Category = "BattleLog")
    static FBattleLogPartyMemberInfo MakeBattleLogPartyMemberInfo(const ACreatureBase* pCreature);

    UFUNCTION(BlueprintPure, Category = "BattleLog")
    static FBattleLogActionInfo MakeBattleLogActionInfo(const AAction_Base* pActionBase);

    UFUNCTION(BlueprintPure, Category = "BattleLog")
    static FBattleLogStatusEffectInfo MakeBattleLogStatusEffectInfo(const FStatusEffectTask& statusEffectActivationTask);

    UFUNCTION(BlueprintPure, Category = "BattleLog")
    static FBattleLogTrapInfo MakeBattleLogTrapInfo(const ATrap_Base* pTrap);

public:

    UFUNCTION(BlueprintCallable, Category = "BattleLog")
    void PushBattleLogItem(UBattleLogItem_Base* pBattleLogItem);

    UFUNCTION(BlueprintCallable, Category = "BattleLog")
    void PushBattleLogItems(TArray<UBattleLogItem_Base*> battleLogItems);

    UFUNCTION(BlueprintCallable, Category = "BattleLog")
    UBattleLogItem_NewTurn* CreateBattleLogItem_NewTurn(int32 turn);

    UFUNCTION(BlueprintCallable, Category = "BattleLog")
    UBattleLogItem_ActionAffectsCreature* CreateBattleLogItem_ActionAffectsCreature(const AAction_Base* pAction,
                                                                                    const ACreatureBase* pTargetCreature);

    UFUNCTION(BlueprintCallable, Category = "BattleLog")
    UBattleLogItem_ActionCancellation* CreateBattleLogItem_ActionCancellation(const AAction_Base* pCancelledAction,
                                                                              const TArray<AAction_Base*>& cancelledByActions);

    UFUNCTION(BlueprintCallable, Category = "BattleLog")
    UBattleLogItem_ActionSummon* CreateBattleLogItem_ActionSummon(const AAction_Base* pAction,
                                                                  const ACreatureBase* pSummonedCreature);

    UFUNCTION(BlueprintCallable, Category = "BattleLog")
    UBattleLogItem_CreatureStatusChange* CreateBattleLogItem_CreatureStatusChange(const ACreatureBase* pCreature,
                                                                                  int32 oldHP,
                                                                                  int32 newHP);

    UFUNCTION(BlueprintCallable, Category = "BattleLog")
    UBattleLogItem_CreatureTrapped* CreateBattleLogItem_CreatureTrapped(const ATrap_Base* pTrap,
                                                                        const ACreatureBase* pTrappedCreature);

    UFUNCTION(BlueprintCallable, Category = "BattleLog")
    UBattleLogItem_DamageCreature* CreateBattleLogItem_DamageCreature(ACreatureBase* attacker,
                                                                      ACreatureBase* target,
                                                                      const FDamageParameters& damage);

    UFUNCTION(BlueprintCallable, Category = "BattleLog")
    UBattleLogItem_HealCreature* CreateBattleLogItem_HealCreature(ACreatureBase* healer,
                                                                  ACreatureBase* target,
                                                                  int32 healedHP);

    UFUNCTION(BlueprintCallable, Category = "BattleLog")
    UBattleLogItem_CreatureMovement* CreateBattleLogItem_CreatureMovement(ACreatureBase* creature,
                                                                          const FIntPoint& fromTile,
                                                                          const FIntPoint& toTile);

    UFUNCTION(BlueprintCallable, Category = "BattleLog")
    UBattleLogItem_RamCreature* CreateBattleLogItem_RamCreature(ACreatureBase* rammer,
                                                                ACreatureBase* target,
                                                                const FIntPoint& ramTile,
                                                                const FDamageParameters& ramDamage);

    UFUNCTION(BlueprintCallable, Category = "BattleLog")
    UBattleLogItem_PushCreature* CreateBattleLogItem_PushCreature(ACreatureBase* pusher,
                                                                  ACreatureBase* target,
                                                                  const FIntPoint& fromTile,
                                                                  const FIntPoint& toTile,
                                                                  const FDamageParameters& pushDamage);

    UFUNCTION(BlueprintCallable, Category = "BattleLog")
    UBattleLogItem_CreatureDeath* CreateBattleLogItem_CreatureDeath(ACreatureBase* creature);

protected:

    TArray<UBattleLogItem_Base*> mBattleLog;
};
