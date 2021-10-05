
#include "Arena/PartyMemberSpawnPoint.h"
#include "Components/ArrowComponent.h"


UPartyMemberSpawnPoint::UPartyMemberSpawnPoint(const FObjectInitializer& objectInitializer)
    : Super(objectInitializer)
{
    PrimaryComponentTick.bCanEverTick = false;
    bHiddenInGame = true;

    // ~!~ NEVER set it to true as we need this data in the shipped builds as well!
    bIsEditorOnly = false;

    /*bWantsInitializeComponent = false;
    bWantsOnUpdateTransform = false;
    bAutoActivate = false;
    bAutoRegister = false;*/
}

void UPartyMemberSpawnPoint::OnRegister()
{
#if WITH_EDITORONLY_DATA
    if (AActor* MyOwner = GetOwner())
    {
        if (mpArrowComponent == nullptr)
        {
            mpArrowComponent = NewObject<UArrowComponent>(MyOwner, NAME_None, RF_Transactional | RF_TextExportTransient);
            mpArrowComponent->SetupAttachment(this);
            mpArrowComponent->SetIsVisualizationComponent(true);
            mpArrowComponent->CreationMethod = CreationMethod;
            mpArrowComponent->RegisterComponentWithWorld(GetWorld());
            
            mpArrowComponent->SetRelativeScale3D(FVector(2.0f, 2.0f, 2.0f));
            mpArrowComponent->SetRelativeRotation(FRotator::MakeFromEuler(FVector(0.0f, 90.0f, 0.0f)));

            UpdateTeamColor();
        }
    }
#endif // #if WITH_EDITORONLY_DATA

    Super::OnRegister();
}

#if WITH_EDITORONLY_DATA
void UPartyMemberSpawnPoint::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
    UpdateTeamColor();
}

void UPartyMemberSpawnPoint::UpdateTeamColor()
{
    FLinearColor teamColor;
    switch (mParty)
    {
    case EParty::Party_AI:
        teamColor = FLinearColor::Red;
        break;
    case EParty::Party_Neutral:
        teamColor.R = 0.3f;
        teamColor.G = 0.3f;
        teamColor.B = 0.3f;
        break;
    case EParty::Party_Player:
        teamColor = FLinearColor::Blue;
        break;
    }
    if (mpArrowComponent != nullptr)
    {
        mpArrowComponent->SetArrowColor(teamColor);
    }
}
#endif // #if WITH_EDITORONLY_DATA