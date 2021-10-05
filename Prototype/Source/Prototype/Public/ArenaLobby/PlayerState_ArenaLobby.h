#pragma once


#include "CoreMinimal.h"
#include "ArenaUnitShopInfo.h"
#include "GameFramework/PlayerState.h"
#include "PlayerState_ArenaLobby.generated.h"


UENUM(BlueprintType)
enum class EPlayerRole : uint8
{
    Admin,
    Client,
    INVALID,
};


UCLASS()
class APlayerState_ArenaLobby
    : public APlayerState
{
    GENERATED_BODY()

public:

    DECLARE_DYNAMIC_MULTICAST_DELEGATE(FEventPlayerStateChanged);

    UPROPERTY(BlueprintAssignable, Category = "ArenaLobby")
    FEventPlayerStateChanged mEventPlayerNameChanged;

    UPROPERTY(BlueprintAssignable, Category = "ArenaLobby")
    FEventPlayerStateChanged mEventUnitsShopChanged;

    UPROPERTY(BlueprintAssignable, Category = "ArenaLobby")
    FEventPlayerStateChanged mEventPartyMembersChanged;

    UPROPERTY(BlueprintAssignable, Category = "ArenaLobby")
    FEventPlayerStateChanged mEventIsReadyChanged;

public:

    UPROPERTY(Replicated, BlueprintReadOnly, Category = "ArenaLobby")
    EPlayerRole mPlayerRole = EPlayerRole::INVALID;

    UPROPERTY(Replicated, BlueprintReadOnly, Category = "ArenaLobby")
    FLinearColor mPartyColor = FLinearColor::Green;

    UPROPERTY(Replicated, BlueprintReadOnly, Category = "ArenaLobby")
    int32 mMoney = 0;

    UPROPERTY(Replicated, BlueprintReadOnly, Category = "ArenaLobby")
    int32 mMaxPartySize = 1;

public:

    void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const;

    void SetUnitsShop(TArray<FArenaUnitShopInfo> unitsShop);

    UFUNCTION(BlueprintPure, Category = "ArenaLobby")
    const TArray<FArenaUnitShopInfo>& GetUnitsShop() const { return mUnitsShop; };

    void AddPartyMember(int32 unitShopIndex);

    void RemPartyMember(int32 partyMemberIndex);

    int32 GetPartyMembersNum() const;

    UFUNCTION(BlueprintPure, Category = "ArenaLobby")
    TArray<TSubclassOf<ACreatureBase>> GetPartyMembersClasses() const;

    void SetIsReady(bool isReady);

    UFUNCTION(BlueprintPure, Category = "ArenaLobby")
    bool GetIsReady() const { return mIsReady; };

protected:

    UPROPERTY(ReplicatedUsing = OnRep_UnitsShop, BlueprintReadOnly, Category = "ArenaLobby")
    TArray<FArenaUnitShopInfo> mUnitsShop;

    UPROPERTY(ReplicatedUsing = OnRep_PartyMembers, BlueprintReadOnly, Category = "ArenaLobby")
    TArray<int32> mPartyMembersShopIndices;

    UPROPERTY(ReplicatedUsing = OnRep_IsReady, BlueprintReadOnly, Category = "ArenaLobby")
    bool mIsReady = false;

protected:

    void OnRep_PlayerName() override;

    UFUNCTION()
    void OnRep_UnitsShop();

    UFUNCTION()
    void OnRep_PartyMembers();

    UFUNCTION()
    void OnRep_IsReady();

};