// Fill out your copyright notice in the Description page of Project Settings.

#include "Arena/TiledMovement.h"
#include "Arena/ArenaUtilities.h"
#include "Arena/ArenaManager.h"
#include "Arena/PassMap.h"
#include "Arena/TurnsManager.h"
#include "Arena/ArenaUnit_Interface.h"
#include "Arena/StatusEffectsManager.h"
#include "Arena/GameMode_ArenaFight.h"
#include "Arena/Damageable.h"
#include "Arena/FireSystem.h"
#include "Arena/GameState_Arena.h"
#include "BitmaskOperator.h"
#include "EngineUtils.h"
#include "UnknownCommon.h"


float UTiledMovement::GetMovementDirectionFinalViewAngle()
{
    return TNumericLimits<float>::Max();
}

UTiledMovement::UTiledMovement(const FObjectInitializer& objectInitializer)
    : Super(objectInitializer)
    , mPhysicalSpeed(100.0f)
    , mAngularSpeed(180.0f)
    , mTileSpeed(1)
    , mSyncPosByTileAtSpawn(true)
    , mOccupationSlot(EOccupationSlot::OccupationSlot_ArenaUnit)
    //, mOccupationMask(APassMap::MakeOccupationBitMask(EOccupationSlot::OccupationSlot_ArenaUnit))
    , mCurTile(0, 0)
    , mViewAngle(0.0f)
    , mFinalViewAngle(0.0f)
    , mTimeToMove(0.0f)
    , mMovementDir(0.0f, 0.0f, 0.0f)
    , mCurrentPhysicalSpeed(0.0f)
    , mTileSpeedBonus(0)
    , mMoving(false)
    , mIsStunned(false)
    , mIsOccupationEnabled(true)
    , mpOwnerStatusEffectsManager(nullptr)
{
    PrimaryComponentTick.bCanEverTick = true;
    bWantsInitializeComponent = true;

    //bAutoActivate = true;
}

void UTiledMovement::InitializeComponent()
{
    Super::ReinitializeProperties();

    //  ~TODO~ Remove ref to damageable component.
    mpOwnerDamageable = GetOwner()->FindComponentByClass<UDamageable>();
}

void UTiledMovement::EnableOccupation(bool enable)
{
    mIsOccupationEnabled = enable;
    //  ~TODO~ When changing this flag - update occupation of the tile in PassMap.
}

bool UTiledMovement::IsOccupationEnabled() const
{
    //  ~TODO~ Remove direct call mpOwnerDamageable.
    return IsActive() && ((mpOwnerDamageable == nullptr && mIsOccupationEnabled) ||
        (mpOwnerDamageable != nullptr && mpOwnerDamageable->IsAlive() && mIsOccupationEnabled));
}

// Called when the game starts
void UTiledMovement::BeginPlay()
{
    Super::BeginPlay();

    UNKNOWN_NOT_ARENA_EDITOR_BEGIN(this)

    if (UArenaUtilities::AreSystemsSetup(this))
    {
        OnSystemsSetup();
    }
    else
    {
        UArenaUtilities::GetArenaManager(this)->mEventSystemsSetup.AddDynamic(this, &UTiledMovement::OnSystemsSetup);
    }

    UNKNOWN_NOT_ARENA_EDITOR_END

    if (GetOwner()->Implements<UArenaUnit_Interface>())
    {
        mpOwnerStatusEffectsManager = IArenaUnit_Interface::Execute_GetStatusEffectsManager(GetOwner());
    }

    mMovementFinalizationRotation = GetOwner()->GetActorRotation();
}

void UTiledMovement::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    UNKNOWN_NOT_ARENA_EDITOR_BEGIN(this)

    if (EndPlayReason == EEndPlayReason::Destroyed &&
        UArenaUtilities::AreSystemsSetup(this))
    {
        UArenaUtilities::GetPassMap(this)->UnregisterPassMapMovement(this);
    }

    UNKNOWN_NOT_ARENA_EDITOR_END

    Super::EndPlay(EndPlayReason);
}

void UTiledMovement::OnSystemsSetup()
{
    UArenaUtilities::GetPassMap(this)->RegisterPassMapMovement(this);

    FTransform initialTransform = GetOwner()->GetActorTransform();
    SyncTileByPos();
    //  ~TODO~ Replace bool with enum with flag "SyncPos_WaitingForSystems".
    if (mSyncPosByTileAtSpawn == false)
    {
        GetOwner()->SetActorTransform(initialTransform);
    }
}

void UTiledMovement::OnMovementStop()
{
    mEventMovementStopped.Broadcast(this);
    mOnStop.Broadcast();
}

void UTiledMovement::OnTileReach()
{
    UE_LOG(ArenaCommonLog, VeryVerbose, TEXT("%s reached tile [%i; %i]."),
           *GetOwner()->GetName(),
           mCurTile.X,
           mCurTile.Y);

    mEventTileReached.Broadcast(this, mCurTile);
}

void UTiledMovement::Move(float DeltaTime)
{
    //	Check if the movement finished:
    if (mPath.Num() == 0)
    {
        if (mMoving)
        {
            UE_LOG(ArenaCommonLog, VeryVerbose, TEXT("%s stopped movement."),
                   *GetOwner()->GetName());

            mMoving = false;

            mMovementFinalizationRotation = mFinalViewAngle == GetMovementDirectionFinalViewAngle() ?
                mMovementDir.ToOrientationRotator() :
                FRotator(0.f, mFinalViewAngle, 0.f);
            
            mCurrentPhysicalSpeed = 0.0f;
            
            OnMovementStop();
        }
        return;
    }
    
    //	If previosly not moving - the movement has just started:
    if (mMoving == false)
    {
        mEventMovementStarted.Broadcast(this);
        mOnStart.Broadcast();
        mMoving = true;
    }

    TActorIterator<APassMap> iter(GetWorld());
    check((bool)iter);

    FVector position = GetOwner()->GetActorLocation();
    FVector nextTilePos = iter->GetTilePositionWorld(mPath.Top());
    FVector toNextTile = nextTilePos - position;
    
    float shift = GetPhysicalSpeed() * DeltaTime;
    if (toNextTile.SizeSquared() <= FMath::Square(shift))
    {
        mCurTile = mPath.Pop();
        shift = (nextTilePos - position).Size();
        position = nextTilePos;
        OnTileReach();
    }
    else
    {
        mMovementDir = toNextTile;
        if (mMovementDir.Normalize() == false)
        {
            UE_LOG(ArenaCommonLog, Warning, TEXT("%s::UTiledMovement::Move() failed to normalize mMovementDir."),
                   *GetOwner()->GetName());
        }
        position += mMovementDir * shift;
    }

    mCurrentPhysicalSpeed = GetPhysicalSpeed();

    GetOwner()->SetActorLocation(position);
}

void UTiledMovement::Rotate(float DeltaTime)
{
    mTargetRotation = IsMoving() || mFinalViewAngle == GetMovementDirectionFinalViewAngle() ? mMovementDir.ToOrientationRotator() : 
        FRotator(0.f, mFinalViewAngle, 0.f);
    GetOwner()->SetActorRotation(FMath::Lerp(GetOwner()->GetActorRotation(), mTargetRotation, 0.085f));
}

APassMap* UTiledMovement::GetPassMap() const
{
    return UArenaUtilities::GetPassMap(GetOwner());
}

void UTiledMovement::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    Move(DeltaTime);

    Rotate(DeltaTime);
}

void UTiledMovement::SetLocation(const FIntPoint& tile)
{
    mCurTile = tile;
    FVector tileLocation = GetPassMap()->GetTilePositionWorld(mCurTile);
    GetOwner()->SetActorLocation(tileLocation);
}

void UTiledMovement::SyncTileByPos()
{
    //  Possibly better to acquire PassMap and register itself as needed to be synced. PassMap stores an array of such actors and 
    //  syncs them at the appropriate time (after setup or at the end of the turn).
    check(UArenaUtilities::AreSystemsSetup(this) && "SyncTileByPos() must be called after all systems are setup!");
    auto pPassMap = GetPassMap();
    auto tile = pPassMap->GetTileInPositionWorld(GetOwner()->GetActorLocation());
    int32 occupationSlotsToBeFree = MakeBitMask(mOccupationSlot) |
        MakeBitMask(EOccupationSlot::OccupationSlot_Static,
                    EOccupationSlot::OccupationSlot_StaticLowObstacle,
                    EOccupationSlot::OccupationSlot_StaticHighObstacle);
    if (pPassMap->IsTileValid(tile) &&
        pPassMap->IsTileFree(tile, occupationSlotsToBeFree))
    {
        mCurTile = tile;
        GetOwner()->SetActorLocation(pPassMap->GetTilePositionWorld(mCurTile));
    }
    else
    {
        UE_LOG(ArenaCommonLog, Error, TEXT("%s can't sync tile by pos. Either invalid or occupied tile was detected."), *GetOwner()->GetName());
    }
}

void UTiledMovement::SetStunned(bool isStunned)
{
    mIsStunned = isStunned;
}

bool UTiledMovement::IsStunned() const
{
    return mIsStunned;
}

bool UTiledMovement::MoveTo(const FIntPoint& targetTile,
                            float finalViewAngle)
{
    if (targetTile == mCurTile)
    {
        return false;
    }

    mPath = GetPassMap()->FindPath(mCurTile,
                                   targetTile,
                                   false,
                                   false,
                                   GetTileSpeed() + 1,
                                   GetTileSpeed());

    //	If no path was detected - movement failed:
    if (mPath.Num() == 0)
    {
        return false;
    }

    Algo::Reverse(mPath);

    mFinalViewAngle = finalViewAngle;

    return true;
}

bool UTiledMovement::MoveToTimed(const FIntPoint& targetTile, 
                                 float timeToProcessMovement, 
                                 float finalViewAngle)
{
    //	If the movement will failed it doesn't matter what value mTimeToMove will get:
    mTimeToMove = timeToProcessMovement;

    return MoveTo(targetTile, finalViewAngle);
}

bool UTiledMovement::MovePath(const TArray<FIntPoint>& pathTiles, 
                              float finalViewAngle)
{
    mPath = pathTiles;

    Algo::Reverse(mPath);

    mFinalViewAngle = finalViewAngle;

    return true;
}

bool UTiledMovement::MovePathTimed(const TArray<FIntPoint>& pathTiles,
                                   float timeToProcessMovement,
                                   float finalViewAngle)
{
    mTimeToMove = timeToProcessMovement;

    return MovePath(pathTiles, finalViewAngle);
}

bool UTiledMovement::TrySetPlacement(const FIntPoint& placement)
{
    APassMap* pPassMap = GetPassMap();
    int32 occupationMask = MakeBitMask(EOccupationSlot::OccupationSlot_Static,
                                       EOccupationSlot::OccupationSlot_ArenaUnit,
                                       EOccupationSlot::OccupationSlot_StaticLowObstacle,
                                       EOccupationSlot::OccupationSlot_StaticHighObstacle);
    if (IsMoving() == false &&
        pPassMap->IsTileFree_Safe(placement, occupationMask))
    {
        mCurTile = placement;
        GetOwner()->SetActorLocation(pPassMap->GetTilePositionWorld(placement));
        return true;
    }
    return false;
}

FIntPoint UTiledMovement::GetCurrentTile() const
{
    return mCurTile;
}

void UTiledMovement::SetPhysicalSpeed(float speed)
{
    mPhysicalSpeed = speed;
}

float UTiledMovement::GetPhysicalSpeed() const
{
    return mPhysicalSpeed;
}

void UTiledMovement::SetAngularSpeed(float speed)
{
    mAngularSpeed = speed;
}

float UTiledMovement::GetAngularSpeed() const
{
    return mAngularSpeed;
}

bool UTiledMovement::IsMoving() const
{
    return mPath.Num() > 0;
}

void UTiledMovement::SetTileSpeed(int32 tilesPerTurn)
{
    check(tilesPerTurn > 0 && "UTiledMovement: trying to set negative or zero tile speed!");
    mTileSpeed = tilesPerTurn;
}

int32 UTiledMovement::GetTileSpeed() const
{
    return FMath::Max(mTileSpeed + mTileSpeedBonus, 0);
}

void UTiledMovement::SetTileSpeedBonus(int32 speedBonus)
{
    mTileSpeedBonus = speedBonus;
}

int32 UTiledMovement::GetTileSpeedBonus() const
{
    return mTileSpeedBonus;
}

void UTiledMovement::RotateTo(float viewAngle)
{
    mFinalViewAngle = viewAngle;

    mMovementFinalizationRotation.Yaw = viewAngle;
    mMovementFinalizationRotation.Roll = 0.0f;
    mMovementFinalizationRotation.Pitch = 0.0f;
}

void UTiledMovement::AimAt(const FVector& aimTarget)
{
    FVector toTarget = aimTarget - GetOwner()->GetActorLocation();
    mMovementFinalizationRotation = toTarget.ToOrientationRotator();
    mMovementFinalizationRotation.Pitch = 0.0f;
    mMovementFinalizationRotation.Roll = 0.0f;
}

void UTiledMovement::AimAtTile(const FIntPoint& aimTargetTile)
{
    FVector aimTarget = GetPassMap()->GetTilePositionWorld(aimTargetTile);
    AimAt(aimTarget);
}

void UTiledMovement::SetOrientation(float viewAngle)
{
    mViewAngle = mFinalViewAngle = viewAngle;
}

float UTiledMovement::GetOrientation() const
{
    return mViewAngle;
}

float UTiledMovement::GetCurrentPhysicalSpeed() const
{
    return mCurrentPhysicalSpeed;
}

void UTiledMovement::StoreState_Inner(ArenaStateContext& context, State_TiledMovement& state) const
{
    state.mCurTile = mCurTile;
    state.mIsStunned = mIsStunned;
    state.mTileSpeed = mTileSpeed;
}

bool UTiledMovement::RestoreState_Inner(const State_TiledMovement& state)
{
    TrySetPlacement(state.mCurTile);
    SetStunned(state.mIsStunned);
    SetTileSpeed(state.mTileSpeed);
    return true;
}

EOccupationSlot UTiledMovement::GetOccupationSlot() const
{
    return mOccupationSlot;
}

/*int32 UTiledMovement::GetOccupationMask() const
{
    return mOccupationMask;
}*/
