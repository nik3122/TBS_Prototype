#pragma once


#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CreatureGhost.generated.h"


class ACreatureBase;
class USceneComponent;


UCLASS(Blueprintable)
class ACreatureGhost
    : public AActor
{
    GENERATED_BODY()

public:

    ACreatureGhost(const FObjectInitializer& initializer = FObjectInitializer::Get());

    void Initialize(ACreatureBase* creature);

protected:

    UPROPERTY(BlueprintReadOnly, Category = "Creature Ghost", Meta = (DisplayName = "AssociatedCreature"))
    ACreatureBase* mAssociatedCreature = nullptr;

protected:

    UFUNCTION(BlueprintImplementableEvent, Meta = (DisplayName = "Initialize"))
    void K2_Initialize();

};