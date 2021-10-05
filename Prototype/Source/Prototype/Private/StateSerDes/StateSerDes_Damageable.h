#pragma once


#include "ArenaStateSerDes.h"
#include "Damageable.h"

/*
template <>
void StateSerDes<State_Damageable>::SerializeXML_Inner(const State_Damageable& state, tinyxml2::XMLPrinter& xmlPrinter)
{
    xmlPrinter.OpenElement("maxhp");
    xmlPrinter.PushText(state.mMaxHP);
    xmlPrinter.CloseElement();
    xmlPrinter.OpenElement("curhp");
    xmlPrinter.PushText(state.mCurrentHP);
    xmlPrinter.CloseElement();
    xmlPrinter.OpenElement("resclear");
    xmlPrinter.PushText(state.mCurrentHP);
    xmlPrinter.CloseElement();
}
*/