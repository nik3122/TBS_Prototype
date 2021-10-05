#pragma once


#include "Arena/ArenaStateSerDes.h"
#include "Arena/CreatureBase.h"


/*template <>
void StateSerDes<State_CreatureBase>::SerializeXML_Inner(const State_CreatureBase& state, tinyxml2::XMLPrinter& xmlPrinter)
{
    xmlPrinter.OpenElement("damageable");
    StateSerDes<decltype(*state.mpDamageableState)>::SerializeXML(*state.mpDamageableState, xmlPrinter);
    xmlPrinter.CloseElement();
}*/

/*
    SerDes for State_CreatureBase.
*/
/*
class StateSerDes_CreatureBase
    : public StateSerDes_Base<State_CreatureBase>
{
    CHECK_CLASS_VERSION(State_CreatureBase, 1, 0);

protected:

    void SerializeXML_Inner(const State_CreatureBase& state, tinyxml2::XMLPrinter& xmlPrinter) final;
};*/