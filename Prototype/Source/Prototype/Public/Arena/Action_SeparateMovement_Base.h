#pragma once


#include "CoreMinimal.h"
#include "Arena/Action_Base.h"
#include "Arena/ArenaUnit_Interface.h"
#include "Action_SeparateMovement_Base.generated.h"


class UHexTilePattern;


UCLASS()
class AAction_SeparateMovement_Base
    : public AAction_Base
{
    GENERATED_BODY()

public:

    AAction_SeparateMovement_Base(const FObjectInitializer& initializer = FObjectInitializer::Get());

    bool Setup() override;

    void Apply();
    
    //  ~TEMP~ Refactoring is needed for Action_Base:
    bool CanBeSetupAt_Implementation(const FIntPoint& potentialSetupTile) const;

    UHexTilePattern* GetTilePattern() const;

protected:

    UPROPERTY(EditAnywhere, Category = "Action", Meta = (DisplayName = "Action Initiative"))
    int32 mActionInitiative = 0;

    UPROPERTY(EditDefaultsOnly, Category = "Action", Meta = (DisplayName = "Tile Pattern"))
    UHexTilePattern* mTilePattern = nullptr;

protected:

    virtual void ApplyInner();

    UFUNCTION(BlueprintImplementableEvent, Meta = (DisplayName = "Apply"))
    void K2_Apply();

    void EnableSetupVisualization_Inner() override;

    void DisableSetupVisualization_Inner() override;

    void UpdateSetupVisualization() override;

#if WITH_EDITOR
    
    void PostEditChangeProperty(FPropertyChangedEvent& propertyChangedEvent) override;

#endif // #if WITH_EDITOR
};
