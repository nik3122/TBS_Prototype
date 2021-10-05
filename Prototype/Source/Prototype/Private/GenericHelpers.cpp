

#include "GenericHelpers.h"


void UGenericHelpers::MarkComponentRenderStateDirty(UActorComponent* pComponent)
{
    pComponent->MarkRenderStateDirty();
}
