#pragma once


#include "ArenaManager_Interface.h"
#include "ArenaManager.generated.h"


class AGameState_Arena;
class ATurnsManager;
class APassMap;
class AActionMap;
class AFireSystem;
class AWeather;
class AParty;


UCLASS()
class AArenaManager
    : public AInfo
    , public IArenaManager_Interface
{
    GENERATED_BODY()

    friend AGameState_Arena;

public:

    DECLARE_DYNAMIC_MULTICAST_DELEGATE(FEventSystemsSetup);
    UPROPERTY(BlueprintAssignable, Category = "Arena")
    FEventSystemsSetup mEventSystemsSetup;

    DECLARE_DELEGATE(FEventArenaSetup);
    FEventArenaSetup mEventArenaSetup;

public:

    AArenaManager(const FObjectInitializer& objectInitializer = FObjectInitializer::Get());

    //  ~?~ This must be called by the system actor which implements GameplaySystem_Interface (not yet introduced).
    //  This notifies AArenaManager that a particular system is ready for setup interactions. Mostly called on BeginPlay.
    void RegisterSystem(AActor* pSystem);

    void SetParties(AParty* pPartyOne, AParty* pPartyTwo);

    bool IsArenaSetup() const;

    //  IArenaManager_Interface::

    AParty* GetPartyOne_Implementation() override;

    AParty* GetPartyTwo_Implementation() override;

    AParty* GetPlayerParty_Implementation() override;

    AParty* GetAIParty_Implementation() override;

    ATurnsManager* GetTurnsManager_Implementation() override;

    APassMap* GetPassMap_Implementation() override;

    AActionMap* GetActionMap_Implementation() override;

    AFireSystem* GetFireSystem_Implementation() override;

    AInfectionSystem* GetInfectionSystem_Implementation() override;

    AWeather* GetWeather_Implementation() override;

    bool AreSystemsSetup_Implementation() const override;

protected:

    UPROPERTY(EditInstanceOnly, Category = "Arena")
    ATurnsManager* mpTurnsManager = nullptr;

    UPROPERTY(EditInstanceOnly, Category = "Arena")
    APassMap* mpPassMap = nullptr;

    UPROPERTY(EditInstanceOnly, Category = "Arena")
    AActionMap* mpActionMap = nullptr;

    UPROPERTY(EditInstanceOnly, Category = "Arena")
    AFireSystem* mpFireSystem = nullptr;

    UPROPERTY(EditInstanceOnly, Category = "Arena")
    AInfectionSystem* mpInfectionSystem = nullptr;

    UPROPERTY(EditInstanceOnly, Category = "Arena")
    AWeather* mpWeather = nullptr;

    bool mAreSystemsSetup = false;

    //  Indicates whether arena sta
    bool mIsArenaSetup = false;

    UPROPERTY(ReplicatedUsing=OnRep_PartyReplicated)
    AParty* mpPartyOne = nullptr;

    UPROPERTY(ReplicatedUsing=OnRep_PartyReplicated)
    AParty* mpPartyTwo = nullptr;

protected:

    UFUNCTION()
    void OnRep_PartyReplicated();

    UFUNCTION()
    void OnActionsSubmittingFinished();

private:

    void BeginPlay() override;

    void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    bool CheckSystemReadinessForSetup(AActor* pSystem) const;

    void TrySetupSystems();

};