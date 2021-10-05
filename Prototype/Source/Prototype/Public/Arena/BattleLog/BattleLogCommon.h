#pragma once


#include "CoreMinimal.h"
#include "Arena/Party.h"
#include "Arena/StatusEffectsManager.h"
#include "BattleLogCommon.generated.h"


class AAction_Base;
class ACreatureBase;
class AStatusEffect;
class ATrap_Base;
class UTexture2D;
struct FDamageParameters;
struct FStatusEffectTask;


USTRUCT(BlueprintType)
struct FBattleLogPartyMemberInfo
{
    GENERATED_BODY()

public:

    UPROPERTY(BlueprintReadWrite, Category = "BattleLog")
    UTexture2D* mpCreatureImage = nullptr;

    UPROPERTY(BlueprintReadWrite, Category = "BattleLog")
    FLinearColor mPartyMemberColor;

    UPROPERTY(BlueprintReadWrite, Category = "BattleLog")
    FArenaPartyInfo mPartyInfo;

public:

    FBattleLogPartyMemberInfo() = default;
    FBattleLogPartyMemberInfo(const ACreatureBase* pCreature);

    void SetFromCreature(const ACreatureBase* pCreature);
};


USTRUCT(BlueprintType)
struct FBattleLogActionInfo
{
    GENERATED_BODY()

public:

    UPROPERTY(BlueprintReadWrite, Category = "BattleLog")
    FBattleLogPartyMemberInfo mExecutorInfo;

    UPROPERTY(BlueprintReadWrite, Category = "BattleLog")
    UTexture2D* mpActionImage = nullptr;

public:

    FBattleLogActionInfo() = default;
    FBattleLogActionInfo(const AAction_Base* pAction);

    void SetFromAction(const AAction_Base* pAction);
};


USTRUCT(BlueprintType)
struct FBattleLogStatusEffectInfo
{
    GENERATED_BODY()

public:

    UPROPERTY(BlueprintReadWrite, Category = "BattleLog")
    TSubclassOf<AStatusEffect> mStatusEffectClass;

    UPROPERTY(BlueprintReadWrite, Category = "BattleLog")
    int32 mDuration = 0;

    UPROPERTY(BlueprintReadWrite, Category = "BattleLog")
    int32 mStage = 0;

public:

    FBattleLogStatusEffectInfo() = default;
    FBattleLogStatusEffectInfo(const FStatusEffectTask& statusEffectActivationTask);

    void SetFromStatusEffectActivationTask(const FStatusEffectTask& statusEffectActivationTask);
};


USTRUCT(BlueprintType)
struct FBattleLogTrapInfo
{
    GENERATED_BODY()

public:

    UPROPERTY(BlueprintReadWrite, Category = "BattleLog")
    FBattleLogPartyMemberInfo mTrapOwnerCreatureInfo;

    UPROPERTY(BlueprintReadWrite, Category = "BattleLog")
    TSubclassOf<ATrap_Base> mTrapClass;

    UPROPERTY(BlueprintReadWrite, Category = "BattleLog")
    FIntPoint mTile;

public:

    FBattleLogTrapInfo() = default;
    FBattleLogTrapInfo(const ATrap_Base* pTrap);

    void SetFromTrap(const ATrap_Base* pTrap);
};