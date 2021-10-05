// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Arena/CreatureBase.h"
#include "GameFramework/Pawn.h"
#include "Math/Color.h"
#include "Party.generated.h"


class ACreatureBase;


UENUM(BlueprintType)
enum class EParty : uint8
{
    Party_Neutral,
    Party_Player,
    Party_AI,
    Party_MAX,
};

USTRUCT(BlueprintType)
struct FArenaPartyInfo
{
    GENERATED_BODY()

public:

    UPROPERTY(BlueprintReadWrite, Category = "Arena")
    FString mPartyName;

    UPROPERTY(BlueprintReadWrite, Category = "Arena")
    FLinearColor mPartyColor;
};


UCLASS(Blueprintable, Category = "Arena")
class PROTOTYPE_API AParty
    : public APawn
{
    GENERATED_BODY()

public:

    /*DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FEventNewMemberJoined, AParty*, pParty, ACreatureBase*, pJoinedCreature);
    UPROPERTY(BlueprintAssignable, BlueprintReadWrite, Category = "Arena")
    FEventNewMemberJoined mEventNewMemberJoined;

    DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FEventMemberRemoved, AParty*, pParty, ACreatureBase*, pRemovedCreature, int32, removedCreatureIndex);
    UPROPERTY(BlueprintAssignable, BlueprintReadWrite, Category = "Arena")
    FEventMemberRemoved mEventMemberRemoved;*/

    DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FEventLastMemberKilled, AParty*, pParty, ACreatureBase*, pLastKilledCreature);
    UPROPERTY(BlueprintAssignable, BlueprintReadWrite, Category = "Arena")
    FEventLastMemberKilled mEventLastMemberKilled;

    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FEventPartyMembersUpdated, AParty*, pParty);
    UPROPERTY(BlueprintAssignable, Category = "Arena")
    FEventPartyMembersUpdated mEventPartyMembersUpdated;

public:

    AParty(const FObjectInitializer& objInit);

    void SetPartyInfo(FArenaPartyInfo partyInfo);

    UFUNCTION(BlueprintPure, Category = "Arena")
    const FArenaPartyInfo& GetPartyInfo() const;

    UFUNCTION(BlueprintPure, Category = "Arena")
    const TArray<ACreatureBase*>& GetMembers();

    UFUNCTION(BlueprintPure, Category = "Arena")
    const TArray<ACreatureBase*>& GetKilledMembers();

    UFUNCTION(BlueprintPure, Category = "Arena")
    ACreatureBase* GetMemberByIndex(int32 index);

    UFUNCTION(BlueprintPure, Category = "Arena")
    ACreatureBase* TryGetMemberByIndex(int32 index);

    UFUNCTION(BlueprintPure, Category = "Arena")
    int32 GetPartySize() const;

    UFUNCTION(BlueprintPure, Category = "Arena")
    bool IsNotEmpty() const;

    UFUNCTION(BlueprintPure, Category = "Arena")
    bool IsIndexValid(int32 index) const;

    UFUNCTION(BlueprintPure, Category = "Arena")
    bool IsInParty(ACreatureBase* pCreature, int32& outMemberIndex);

    UFUNCTION(BlueprintPure, Category = "Arena")
    bool GetMemberIndexIfExists(ACreatureBase* pCreature,
                                int32& outIndex);

    UFUNCTION(BlueprintCallable, Category = "Arena")
    void AddMember(ACreatureBase* pCreature);

    UFUNCTION(BlueprintCallable, Category = "Arena")
    void RemoveMemberByIndex(int32 memberIndex);

    UFUNCTION(BlueprintCallable, Category = "Arena")
    void RemoveMemeberIfExists(ACreatureBase* pCreature);

    UFUNCTION(BlueprintPure, Category = "Arena")
    bool IsPartyDead() const;

protected:

    UPROPERTY(ReplicatedUsing=OnRep_Members)
    TArray<ACreatureBase*> mMembers;

    UPROPERTY(Replicated)
    TArray<ACreatureBase*> mKilledMembers;

    UPROPERTY(Replicated)
    FArenaPartyInfo mPartyInfo;

protected:

    void BeginPlay() override;

    void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    UFUNCTION()
    void OnMemberKilled(UDamageable* pDamageable);

    UFUNCTION()
    void OnMemberDestroyed(AActor* pDestroyedActor);

    UFUNCTION()
    void OnRep_Members();

private:

    static FLinearColor sPartyMemberColors[];
};
