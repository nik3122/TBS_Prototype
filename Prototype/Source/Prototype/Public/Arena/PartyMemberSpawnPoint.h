#pragma once


#include "CreatureBase.h"
#include "Party.h"
#include "PartyMemberSpawnPoint.generated.h"


class UArrowComponent;


UCLASS(meta = (BlueprintSpawnableComponent))
class UPartyMemberSpawnPoint
    : public USceneComponent
{
    GENERATED_BODY()

    friend class APassMap;

public:

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Unknown|Arena")
    ECreatureClass mPriorCreatureClass;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Unknown|Arena")
    EParty mParty = EParty::Party_Neutral;

public:

    UPartyMemberSpawnPoint(const FObjectInitializer& objectInitializer = FObjectInitializer::Get());

protected:

#if WITH_EDITORONLY_DATA
    UArrowComponent* mpArrowComponent = nullptr;
#endif // #if WITH_EDITORONLY_DATA

protected:

    void OnRegister() override;

#if WITH_EDITORONLY_DATA
    void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
    void UpdateTeamColor();
#endif // #if WITH_EDITORONLY_DATA
};