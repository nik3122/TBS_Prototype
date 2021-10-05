// Fill out your copyright notice in the Description page of Project Settings.

#include "Arena/CreatureBase.h"
#include "Arena/ActionManager.h"
#include "Arena/AnimationManager.h"
#include "Arena/ArenaManager.h"
#include "Arena/ArenaUnitAttributes.h"
#include "Arena/ArenaUtilities.h"
#include "Arena/BattleLog/BattleLog.h"
#include "Arena/BattleLog/BattleLogItem_CreatureDeath.h"
#include "Arena/CreatureGhost.h"
#include "Arena/Damageable.h"
#include "Arena/SocialComponent.h"
#include "Arena/StatusEffectsManager.h"
#include "Arena/TiledMovement.h"
#include "Arena/TurnsManager.h"
#include "Arena/WeaponOperator.h"
#include "Net/UnrealNetwork.h"
#include "Kismet/GameplayStatics.h"


ACreatureBase::ACreatureBase(const FObjectInitializer& objectInitializer)
    : Super(objectInitializer)
    , mIsStunned(false)
{
    //	~!~ ~?~ Each created component should be registered, isn't it?

    PrimaryActorTick.bCanEverTick = false;

    AutoPossessAI = EAutoPossessAI::Disabled;

    //	Creating skeletal mesh component:
    mpSkeletalMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SkeletalMesh"));
    //mpSkeletalMesh->SetupAttachment(RootComponent);
    RootComponent = mpSkeletalMesh;

    //	Creating damageable component:
    mpDamageable = CreateDefaultSubobject<UDamageable>(TEXT("Damageable"));
    mpDamageable->mEventDeath.AddDynamic(this, &ACreatureBase::OnDeath);
    mpDamageable->mEventRevive.AddDynamic(this, &ACreatureBase::OnRevive);
    mpDamageable->SetAutoActivate(true);

    //  Visual damageable component:
    mVisualDamageable = CreateDefaultSubobject<UDamageable>(TEXT("VisualDamageable"));
    mVisualDamageable->mEventDeath.AddDynamic(this, &ACreatureBase::OnVisualDeath);
    mVisualDamageable->mEventRevive.AddDynamic(this, &ACreatureBase::OnVisualRevive);
    mVisualDamageable->SetAutoActivate(true);

    //	Creating tiled movement component:
    mpMovement = CreateDefaultSubobject<UTiledMovement>(TEXT("TiledMovement"));
    mpMovement->SetTileSpeed(1);
    mpMovement->SetAutoActivate(true);

    //	Action manager component:
    mpActionManager = CreateDefaultSubobject<UActionManager>(TEXT("ActionManager"));
    mpActionManager->SetIsReplicated(true);
    mpActionManager->SetAutoActivate(true);

    mpStatusEffectsManager = CreateDefaultSubobject<UStatusEffectsManager>(TEXT("StatusEffectsManager"));
    mpStatusEffectsManager->SetAutoActivate(true);

    mpAnimationManager = CreateDefaultSubobject<UAnimationManager>(TEXT("UAnimationManager"));
    mpAnimationManager->SetAutoActivate(true);

    mpWeaponOperator = CreateDefaultSubobject<UWeaponOperator>(TEXT("WeaponOperator"));
    mpWeaponOperator->SetAutoActivate(true);

    mpSocialComponent = CreateDefaultSubobject<USocialComponent>(TEXT("SocialComponent"));
    mpSocialComponent->SetIsReplicated(true);
    mpSocialComponent->SetAutoActivate(true);

    mArenaUnitAttributes = CreateDefaultSubobject<UArenaUnitAttributes>(TEXT("ArenaUnitAttributes"));

    bReplicates = true;
    bAlwaysRelevant = true;
    SetReplicateMovement(false);
}

void ACreatureBase::PostInitializeComponents()
{
    Super::PostInitializeComponents();

    mVisualDamageable->DuplicateState(mpDamageable);
}

UTexture2D* ACreatureBase::GetCreatureImage() const
{
    return mpCreatureImage;
}

void ACreatureBase::SetPartyMemberColor(const FLinearColor& color)
{
    mPartyMemberColor = color;
    if (HasAuthority())
    {
        K2_OnPartyMemberColorChanged();
    }
}

const FLinearColor& ACreatureBase::GetPartyMemberColor() const
{
    return mPartyMemberColor;
}

void ACreatureBase::BeginPlay()
{
    Super::BeginPlay();

    FOnSystemsSetupDelegate delegate;
    delegate.BindDynamic(this, &ACreatureBase::OnSystemsSetup);
    UArenaUtilities::ExecuteWhenSystemsSetup(this, delegate);
}

void ACreatureBase::OnSystemsSetup()
{
    ATurnsManager* turnsManager = UArenaUtilities::GetTurnsManager(this);
    turnsManager->mEventFinalizeTurn.AddDynamic(this, &ACreatureBase::OnFinalizeTurn);
}

void ACreatureBase::OnFinalizeTurn()
{
    mVisualDamageable->DuplicateState(mpDamageable);
}

void ACreatureBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(ACreatureBase, mPartyMemberColor);
}

ACreatureGhost* ACreatureBase::GetCreatureGhost() const
{
    if (mCreatureGhost == nullptr)
    {
        const_cast<ACreatureBase*>(this)->SpawnCreatureGhost();
    }
    return mCreatureGhost;
}

void ACreatureBase::SetStunned(bool isStunned)
{
    mIsStunned = isStunned;
}

bool ACreatureBase::IsStunned() const
{
    return mIsStunned;
}

UTiledMovement* ACreatureBase::GetTiledMovementComponent()
{
    return mpMovement;
}

UDamageable* ACreatureBase::GetDamageableComponent()
{
    return mpDamageable;
}

UActionManager* ACreatureBase::GetActionManagerComponent()
{
    return mpActionManager;
}

UStatusEffectsManager* ACreatureBase::GetStatusEffectsManagerComponent()
{
    return mpStatusEffectsManager;
}

UArenaUnitAttributes* ACreatureBase::GetArenaUnitAttributesComponent() const
{
    return mArenaUnitAttributes;
}

UDamageable* ACreatureBase::GetDamageable() const
{
    return mpDamageable;
}

UDamageable* ACreatureBase::GetVisualDamageable() const
{
    return mVisualDamageable;
}

UActionManager* ACreatureBase::GetActionManager() const
{
    return mpActionManager;
}

UTiledMovement* ACreatureBase::GetTiledMovement() const
{
    return mpMovement;
}

UStatusEffectsManager* ACreatureBase::GetStatusEffectsManager() const
{
    return mpStatusEffectsManager;
}

UAnimationManager* ACreatureBase::GetAnimationManager() const
{
    return mpAnimationManager;
}

UWeaponOperator* ACreatureBase::GetWeaponOperator() const
{
    return mpWeaponOperator;
}

USocialComponent* ACreatureBase::GetSocialComponent() const
{
    return mpSocialComponent;
}

UArenaUnitAttributes* ACreatureBase::GetArenaUnitAttributes() const
{
    return mArenaUnitAttributes;
}

void ACreatureBase::SetIsFocusedPartyMember(bool isFocused)
{
    mIsFocusedPartyMember = isFocused;
    K2_OnSetIsFocusedPartyMember(mIsFocusedPartyMember);
}

UDamageable* ACreatureBase::GetDamageable_Implementation()
{
    return mpDamageable;
}

UDamageable* ACreatureBase::GetVisualDamageable_Implementation()
{
    return mVisualDamageable;
}

UActionManager* ACreatureBase::GetActionManager_Implementation()
{
    return mpActionManager;
}

UTiledMovement* ACreatureBase::GetTiledMovement_Implementation()
{
    return mpMovement;
}

UStatusEffectsManager* ACreatureBase::GetStatusEffectsManager_Implementation()
{
    return mpStatusEffectsManager;
}

UAnimationManager* ACreatureBase::GetAnimationManager_Implementation()
{
    return mpAnimationManager;
}

UWeaponOperator* ACreatureBase::GetWeaponOperator_Implementation()
{
    return mpWeaponOperator;
}

USocialComponent* ACreatureBase::GetSocialComponent_Implementation()
{
    return mpSocialComponent;
}

UArenaUnitAttributes* ACreatureBase::GetArenaUnitAttributes_Implementation()
{
    return mArenaUnitAttributes;
}

namespace
{
    const char* pNodeName_Type = "type";
    const char* pNodeName_Damageable = "damageable";
}

void ACreatureBase::OnDeath(UDamageable* pDamageable)
{
    mpMovement->SetActive(false);
    mpActionManager->SetActive(false);
    mpStatusEffectsManager->SetActive(false);
}

void ACreatureBase::OnRevive(UDamageable* pDamageable)
{
    mpMovement->SetActive(true);
    mpActionManager->SetActive(true);
    mpStatusEffectsManager->SetActive(true);
}

void ACreatureBase::OnVisualDeath(UDamageable* visualDamageable)
{
    UBattleLog* battleLog = UArenaUtilities::GetBattleLog(this);
    auto battleLogItem = battleLog->CreateBattleLogItem_CreatureDeath(this);
    battleLog->PushBattleLogItem(battleLogItem);
}

void ACreatureBase::OnVisualRevive(UDamageable* visualDamageable)
{
    // ?
}

void ACreatureBase::StoreState_Inner(ArenaStateContext& context, State_CreatureBase& state) const
{
    FString className = GetClass()->GetName();
    auto classNameANSII = StringCast<ANSICHAR>(*className);
    //  ~!~ Somehow size of the converted string is one char fewer than origin string:
    state.mpClassName = context.AllocText(classNameANSII.Get(), classNameANSII.Length() + 1);
    state.mClassNameSize = classNameANSII.Length();
    state.mpDamageableState = mpDamageable->StoreState(context);
    state.mpTiledMovementState = mpMovement->StoreState(context);
    state.mpStatusEffectsManagerState = mpStatusEffectsManager->StoreState(context);
    state.mpActionManagerState = mpActionManager->StoreState(context);
}

bool ACreatureBase::RestoreState_Inner(const State_CreatureBase& state)
{
    return true;
}

void ACreatureBase::SpawnCreatureGhost()
{
    mCreatureGhost = GetWorld()->SpawnActorDeferred<ACreatureGhost>(mCreatureGhostClass,
                                                                    GetActorTransform());
    mCreatureGhost->Initialize(this);
    UGameplayStatics::FinishSpawningActor(mCreatureGhost,
                                          GetActorTransform());
}

void ACreatureBase::OnRep_PartyMemberColor()
{
    K2_OnPartyMemberColorChanged();
}
