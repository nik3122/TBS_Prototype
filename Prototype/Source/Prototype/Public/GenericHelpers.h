#pragma once


#include "GenericHelpers.generated.h"


UCLASS()
class UGenericHelpers
    : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:

    UFUNCTION(BlueprintCallable)
    static void MarkComponentRenderStateDirty(UActorComponent* pComponent);
};