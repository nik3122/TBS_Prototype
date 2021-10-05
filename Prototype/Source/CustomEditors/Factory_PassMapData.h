// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Factories/Factory.h"
#include "Factory_PassMapData.generated.h"

/**

*/
UCLASS()
class CUSTOMEDITORS_API UFactory_PassMapData : public UFactory
{
    GENERATED_BODY()
    
public:

    UFactory_PassMapData(const class FObjectInitializer& initializer);

    virtual uint32 GetMenuCategories() const override;

    virtual bool FactoryCanImport(const FString& filename) override;

    virtual UObject* FactoryCreateText(UClass* inClass,
                                       UObject* inParent,
                                       FName inName,
                                       EObjectFlags flags,
                                       UObject* context,
                                       const TCHAR* type,
                                       const TCHAR*& buffer,
                                       const TCHAR* bufferEnd,
                                       FFeedbackContext* feedback) override;

    UObject* FactoryCreateNew(UClass* InClass,
                              UObject* InParent,
                              FName InName,
                              EObjectFlags Flags,
                              UObject* Context,
                              FFeedbackContext* Warn,
                              FName CallingContext) override;
};
