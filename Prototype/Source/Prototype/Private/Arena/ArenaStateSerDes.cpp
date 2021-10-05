
#include "Arena/ArenaStateSerDes.h"

#include "Arena/Damage.h"
#include "Arena/GameState_Arena.h"
#include "Arena/CreatureBase.h"
#include "Arena/Damageable.h"
#include "Arena/Firesystem.h"
#include "Arena/StatusEffect.h"
#include "Arena/StatusEffectsManager.h"
#include "Arena/Action_Base.h"
#include "Arena/ActionManager.h"
#include "Arena/TiledMovement.h"
#include "Arena/GameMode_ArenaFight.h"


//  FDamageParameters::

template <>
void StateSerDes<FDamageParameters>::SerializeXML_Inner(const FDamageParameters& state, tinyxml2::XMLPrinter& xmlPrinter)
{
    CHECK_CLASS_VERSION(FDamageParameters, 1, 0);

    xmlPrinter.OpenElement("physical");
    xmlPrinter.PushText(state.mPhysical);
    xmlPrinter.CloseElement();
    xmlPrinter.OpenElement("poison");
    xmlPrinter.PushText(state.mPoison);
    xmlPrinter.CloseElement();
    xmlPrinter.OpenElement("magical");
    xmlPrinter.PushText(state.mMagical);
    xmlPrinter.CloseElement();
    xmlPrinter.OpenElement("fire");
    xmlPrinter.PushText(state.mFire);
    xmlPrinter.CloseElement();
}

//  State_Damageable::

template <>
void StateSerDes<State_Damageable>::SerializeXML_Inner(const State_Damageable& state, tinyxml2::XMLPrinter& xmlPrinter)
{
    CHECK_CLASS_VERSION(State_Damageable, 1, 0);

    xmlPrinter.OpenElement("maxhp");
    xmlPrinter.PushText(state.mMaxHP);
    xmlPrinter.CloseElement();
    xmlPrinter.OpenElement("curhp");
    xmlPrinter.PushText(state.mCurrentHP);
    xmlPrinter.CloseElement();

    xmlPrinter.OpenElement("resclear");
    StateSerDes<FDamageParameters>::SerializeXML(state.mResistClear, xmlPrinter);
    xmlPrinter.CloseElement();
    xmlPrinter.OpenElement("resbonus");
    StateSerDes<FDamageParameters>::SerializeXML(state.mResistBonus, xmlPrinter);
    xmlPrinter.CloseElement();
    xmlPrinter.OpenElement("defclear");
    StateSerDes<FDamageParameters>::SerializeXML(state.mDefenceClear, xmlPrinter);
    xmlPrinter.CloseElement();
    xmlPrinter.OpenElement("defbonus");
    StateSerDes<FDamageParameters>::SerializeXML(state.mDefenceBonus, xmlPrinter);
    xmlPrinter.CloseElement();

    xmlPrinter.OpenElement("immortal");
    xmlPrinter.PushText(state.mIsImmortal);
    xmlPrinter.CloseElement();
}


//  State_StatusEffect::

template <>
void StateSerDes<State_StatusEffect>::SerializeXML_Inner(const State_StatusEffect& state, tinyxml2::XMLPrinter& xmlPrinter)
{
    CHECK_CLASS_VERSION(State_StatusEffect, 1, 1);

    check(state.mpClassName != nullptr);
    xmlPrinter.PushAttribute("class", state.mpClassName);

    xmlPrinter.OpenElement("timer");
        xmlPrinter.PushText(state.mTurnsTimer);
    xmlPrinter.CloseElement();
}


//  State_StatusEffectsManager::

template <>
void StateSerDes<State_StatusEffectsManager>::SerializeXML_Inner(const State_StatusEffectsManager& state, tinyxml2::XMLPrinter& xmlPrinter)
{
    CHECK_CLASS_VERSION(State_StatusEffectsManager, 1, 0);

    for (int32 i = 0; i < state.mStatusEffectsCount; ++i)
    {
        xmlPrinter.OpenElement("effect");
        StateSerDes<State_StatusEffect>::SerializeXML(*state.mppStatusEffectsStates[i], xmlPrinter);
        xmlPrinter.CloseElement();
    }
}


//  State_TiledMovement::

template <>
void StateSerDes<State_TiledMovement>::SerializeXML_Inner(const State_TiledMovement& state, tinyxml2::XMLPrinter& xmlPrinter)
{
    CHECK_CLASS_VERSION(State_TiledMovement, 1, 0);

    xmlPrinter.OpenElement("tile");
        xmlPrinter.PushText(state.mCurTile.X);
        xmlPrinter.PushText(",");
        xmlPrinter.PushText(state.mCurTile.Y);
    xmlPrinter.CloseElement();

    xmlPrinter.OpenElement("speed");
        xmlPrinter.PushText(state.mTileSpeed);
    xmlPrinter.CloseElement();

    xmlPrinter.OpenElement("stun");
        xmlPrinter.PushText(state.mIsStunned);
    xmlPrinter.CloseElement();
}


//  State_Action_Base::

template <>
void StateSerDes<State_Action_Base>::SerializeXML_Inner(const State_Action_Base& state, tinyxml2::XMLPrinter& xmlPrinter)
{
    CHECK_CLASS_VERSION(State_Action_Base, 1, 1);

    xmlPrinter.PushAttribute("class", state.mpClassName);

    xmlPrinter.PushAttribute("setup", state.mIsSetup);

    xmlPrinter.OpenElement("setup");
        for (int32 i = 0; i < state.mSetupTilesCount; ++i)
        {
            FIntPoint tile = state.mSetupTiles[i];
            xmlPrinter.PushText(tile.X);
            xmlPrinter.PushText(",");
            xmlPrinter.PushText(tile.Y);
            xmlPrinter.PushText(";");
        }
    xmlPrinter.CloseElement();
}


//  State_ActionManager::

template <>
void StateSerDes<State_ActionManager>::SerializeXML_Inner(const State_ActionManager& state, tinyxml2::XMLPrinter& xmlPrinter)
{
    CHECK_CLASS_VERSION(State_ActionManager, 1, 0);

    if (state.mActionsCount > 0)
    {
        check(state.mppActionsStates != nullptr);
        for (int32 i = 0; i < state.mActionsCount; ++i)
        {
            xmlPrinter.OpenElement("action");
            check(state.mppActionsStates[i] != nullptr);
            StateSerDes<State_Action_Base>::SerializeXML(*state.mppActionsStates[i], xmlPrinter);
            xmlPrinter.CloseElement();
        }
    }

    xmlPrinter.OpenElement("toexecute");
        xmlPrinter.PushText(state.mActionToExecuteIndex);
    xmlPrinter.CloseElement();

    xmlPrinter.OpenElement("reaction");
        xmlPrinter.OpenElement("nominal");
        xmlPrinter.PushText(state.mNominalReactionRate);
        xmlPrinter.CloseElement();
        xmlPrinter.OpenElement("bonus");
        xmlPrinter.PushText(state.mReactionRateBonus);
        xmlPrinter.CloseElement();
    xmlPrinter.CloseElement();
}


//  State_CreatureBase::

template <>
void StateSerDes<State_CreatureBase>::SerializeXML_Inner(const State_CreatureBase& state, tinyxml2::XMLPrinter& xmlPrinter)
{
    CHECK_CLASS_VERSION(State_CreatureBase, 1, 0);

    check(state.mpClassName != nullptr);
    xmlPrinter.PushAttribute("class", state.mpClassName);

    xmlPrinter.OpenElement("damageable");
    StateSerDes<State_Damageable>::SerializeXML(*state.mpDamageableState, xmlPrinter);
    xmlPrinter.CloseElement();

    xmlPrinter.OpenElement("tiledmovement");
    StateSerDes<State_TiledMovement>::SerializeXML(*state.mpTiledMovementState, xmlPrinter);
    xmlPrinter.CloseElement();

    xmlPrinter.OpenElement("statuseffectsmanager");
    StateSerDes<State_StatusEffectsManager>::SerializeXML(*state.mpStatusEffectsManagerState, xmlPrinter);
    xmlPrinter.CloseElement();

    xmlPrinter.OpenElement("actionmanager");
    StateSerDes<State_ActionManager>::SerializeXML(*state.mpActionManagerState, xmlPrinter);
    xmlPrinter.CloseElement();
}


//  PassMap:
template <>
void StateSerDes<State_PassMap>::SerializeXML_Inner(const State_PassMap& state, tinyxml2::XMLPrinter& xmlPrinter)
{
    CHECK_CLASS_VERSION(State_PassMap, 1, 0);

    xmlPrinter.PushAttribute("x", state.mSizeX);
    xmlPrinter.PushAttribute("y", state.mSizeY);
    xmlPrinter.OpenElement("occupations");

    //std::size_t binaryDataSize = sizeof(State_PassMap::OccupiedTile) * state.mOccupiedTilesCount;
    //xmlPrinter.PushText(reinterpret_cast<const char*>(state.mpOccupiedTiles), binaryDataSize);

    //  ~!~HACK~ We change increase each occupation mask by 1 for not getting char == 0 (EOF):
    int32 tilesCount = state.mSizeX * state.mSizeY;
    char* pTilesOccupations = new char[tilesCount];
    FMemory::Memcpy(pTilesOccupations, state.mpTilesOccupations, tilesCount);
    for (int32 i = 0; i < tilesCount; ++i)
    {
        //  ~!~ Obviously as we never can have 0b11111111 mask we won't get 0b00000000 after this operation.
        pTilesOccupations[i] += 1;
    }
    xmlPrinter.PushText(pTilesOccupations, static_cast<std::size_t>(tilesCount));
    xmlPrinter.CloseElement();
}


//  Firesystem:

template <>
void StateSerDes<State_FireSystem>::SerializeXML_Inner(const State_FireSystem& state, tinyxml2::XMLPrinter& xmlPrinter)
{
    CHECK_CLASS_VERSION(State_FireSystem, 1, 0);

    xmlPrinter.OpenElement("toprocess");
    xmlPrinter.PushAttribute("size", state.mTilesToProcessCount);
    for (int32 i = 0; i < state.mTilesToProcessCount; ++i)
    {
        if (i != 0) xmlPrinter.PushText(",");
        xmlPrinter.PushText(state.mpTilesToProcess[i]);
    }
    xmlPrinter.CloseElement();

    xmlPrinter.OpenElement("onfire");
    xmlPrinter.PushAttribute("size", state.mTilesOnFireCount);
    for (int32 i = 0; i < state.mTilesOnFireCount; ++i)
    {
        if (i != 0) xmlPrinter.PushText(",");
        xmlPrinter.PushText(state.mpTilesOnFire[i]);
    }
    xmlPrinter.CloseElement();
}

//  State_Arena::

template <>
void StateSerDes<State_Arena>::SerializeXML_Inner(const State_Arena& state, tinyxml2::XMLPrinter& xmlPrinter)
{
    CHECK_CLASS_VERSION(State_Arena, 1, 0);

    xmlPrinter.PushAttribute("turn", state.mTurn);

    xmlPrinter.OpenElement("player");
    for (int32 i = 0; i < state.mPlayerPartyMembersStatesCount; ++i)
    {
        xmlPrinter.OpenElement("creature");
        StateSerDes<State_CreatureBase>::SerializeXML(*state.mppPlayerPartyMembersStates[i], xmlPrinter);
        xmlPrinter.CloseElement();
    }
    xmlPrinter.CloseElement();

    xmlPrinter.OpenElement("ai");
    for (int32 i = 0; i < state.mAIPartyMembersStatesCount; ++i)
    {
        xmlPrinter.OpenElement("creature");
        StateSerDes<State_CreatureBase>::SerializeXML(*state.mppAIPartyMembersStates[i], xmlPrinter);
        xmlPrinter.CloseElement();
    }
    xmlPrinter.CloseElement();

    xmlPrinter.OpenElement("firesystem");
    StateSerDes<State_FireSystem>::SerializeXML(*state.mpFireSystemState, xmlPrinter);
    xmlPrinter.CloseElement();

    xmlPrinter.OpenElement("passmap");
    StateSerDes<State_PassMap>::SerializeXML(*state.mpPassMapState, xmlPrinter);
    xmlPrinter.CloseElement();
}
