#pragma once


//#include "tinyxml2.h"
#include "Third Party/TinyXML/tinyxml2.h"

#include "ArenaState.h"


/*
    StateSerDes Serializer/Deserializer
*/
template <class State>
class StateSerDes
{
public:

    StateSerDes() = delete;
    StateSerDes(const StateSerDes&) = delete;
    StateSerDes(StateSerDes&&) = delete;

    //static StateSerDes& GetInstance() = 0;

    //  StateSerDes_Interface::

    static void SerializeXML(const State& state, tinyxml2::XMLPrinter& xmlPrinter);
    //void DeserializeXML(const tinyxml2::XMLNode* pNode, State_Base& state);

private:

    static void SerializeXML_Inner(const State& state, tinyxml2::XMLPrinter& xmlPrinter);
    //virtual void DeserializeXML_Inner(const tinyxml2::XMLNode* pNode, State_Base*& state) = 0;
};

template <class State>
void StateSerDes<State>::SerializeXML(const State& state, tinyxml2::XMLPrinter& xmlPrinter)
{
    xmlPrinter.PushAttribute("major", CLASS_VERSION_MAJOR(State));
    xmlPrinter.PushAttribute("minor", CLASS_VERSION_MINOR(State));

    SerializeXML_Inner(static_cast<const State&>(state), xmlPrinter);
}


/*
template <class State>
class StateSerDes_Base
{
public:

    StateSerDes_Base(const StateSerDes_Base&) = delete;
    StateSerDes_Base(StateSerDes_Base&&) = delete;

    static StateSerDes_Base& GetInstance() = 0;

    //  StateSerDes_Interface::

    void SerializeXML(const State_Base& state, tinyxml2::XMLPrinter& xmlPrinter);
    //void DeserializeXML(const tinyxml2::XMLNode* pNode, State_Base& state);

protected:

    StateSerDes_Base() = default;

    void SerializeXML_Inner(const State& state, tinyxml2::XMLPrinter& xmlPrinter);
    //virtual void DeserializeXML_Inner(const tinyxml2::XMLNode* pNode, State_Base*& state) = 0;
};

template <class State>
void StateSerDes_Base<State>::SerializeXML(const State_Base& state, tinyxml2::XMLPrinter& xmlPrinter)
{
    xmlPrinter.PushAttribute("major", CLASS_VERSION_MAJOR(State));
    xmlPrinter.PushAttribute("minor", CLASS_VERSION_MINOR(State));

    SerializeXML_Inner(static_cast<const State&>(state), xmlPrinter);
}


class StateSerDesManager
{
public:

    StateSerDesManager(const StateSerDesManager&) = delete;
    StateSerDesManager(StateSerDesManager&&) = delete;

    StateSerDesManager& GetInstance();

    template <class State, class SerDes>
    void RegisterStateSerDes();

    template <class State>
    StateSerDes_Base<State>& GetStateSerDes();

private:

    template <class State>
    struct Registry
    {
        static void* spSerDesInstance = nullptr;
    };

private:

    static StateSerDesManager* spInstance;

private:

    StateSerDesManager() = default;
};


template <class State, class SerDes>
void StateSerDesManager::RegisterStateSerDes()
{
    Registry<State>::spSerDesInstance = SerDes::GetInstance();
}

template <class State>
StateSerDes_Base<State>& StateSerDesManager::GetStateSerDes()
{
    Registry<State>::spSerDesInstance = SerDes::GetInstance();
}
*/