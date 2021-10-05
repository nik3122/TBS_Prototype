

#include "Arena/CreatureGhost.h"
#include "Components/SceneComponent.h"


ACreatureGhost::ACreatureGhost(const FObjectInitializer& initializer)
    : Super(initializer)
{
    RootComponent = CreateDefaultSubobject<USceneComponent>("Root");
}

void ACreatureGhost::Initialize(ACreatureBase* creature)
{
    check(HasActorBegunPlay() == false);

    mAssociatedCreature = creature;

    K2_Initialize();
}
