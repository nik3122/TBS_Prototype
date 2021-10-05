#pragma once


#include "Action_Spawn_Base.h"
#include "CreatureBase.h"
#include "Action_Summon_Base.generated.h"


class ACreatureBase;


UCLASS(Abstract)
class AAction_Summon_Base
    : public AAction_Spawn_Base
{
    GENERATED_BODY()

public:

    AAction_Summon_Base(const FObjectInitializer& objectInitializer = FObjectInitializer::Get());

    void PreExecutionApply_Implementation() override;

protected:

    UPROPERTY(EditDefaultsOnly, Category = "Arena")
    TSubclassOf<ACreatureBase> mCreatureToSpawnClass;

protected:

    void OnConstruction(const FTransform& transform) override;

};