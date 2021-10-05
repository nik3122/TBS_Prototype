#pragma once


#include "CoreMinimal.h"
#include "Version.h"


struct State_Base;
template <class State>
class ArenaStateObject_Base;
class ArenaStateContext;


struct State_Base
{
    //
};


/*
    Base class for objects which can store/restore their states. States of such classes are included into Arena State.
    
    Stored Arena State is used for primary debug purposes. Serialized Arena State is included into bug report.
*/
template <class State>
class ArenaStateObject_Base
{
public:

    State* StoreState(ArenaStateContext& context) const;

    bool RestoreState(const State* pState);

    //bool ValidateState(State_Base* pState) const;

protected:

    virtual void StoreState_Inner(ArenaStateContext& context, State& state) const = 0;

    virtual bool RestoreState_Inner(const State& state) = 0;
};

template <class State>
State* ArenaStateObject_Base<State>::StoreState(ArenaStateContext& context) const
{
    State* pState = context.AllocMemory<State>(1);
    StoreState_Inner(context, *pState);
    return pState;
}

template <class State>
bool ArenaStateObject_Base<State>::RestoreState(const State* pState)
{
    check(pState != nullptr);
    return RestoreState_Inner(*pState);
}


/*
    ArenaStateContext is a storage for stored states of arena objects.
    It offers memory pool for allocating states and states' data.
    Whole single arena state hierarchy should be stored in a single ArenaStateContext.

    NOTE:
        Storing states don't use new or other dynamic allocations except one proposed by ArenaStateContext.

    ~?~RENAME~ ArenaStateStorage
*/
class ArenaStateContext
{
public:

    static const SIZE_T sMemoryPoolSize = 8192;
    static const SIZE_T sMemoryAligment = 16;
    static const SIZE_T sReallocStep = 4096;

    using byte = unsigned char;

public:

    ArenaStateContext();

    ~ArenaStateContext();

    template <class T>
    T* AllocMemory(SIZE_T elementsCount = 1);

    template <class CharType>
    CharType* AllocText(const CharType* pSrc, int32 textSize);

    void Reset();

private:

    SIZE_T mMemoryPoolSize;
    byte* mpMemoryPool;
    byte* mpAllocIterator;

private:

    void ReallocMemoryPool(SIZE_T newMemoryPoolSize);
};

template <class CharType>
CharType* ArenaStateContext::AllocText(const CharType* pSrc, int32 textSize)
{
    CharType* pDst = AllocMemory<CharType>(textSize);
    FMemory::Memcpy(pDst, pSrc, textSize);
    return pDst;
}

template <class T>
T* ArenaStateContext::AllocMemory(SIZE_T elementsCount)
{
    //return new T[elementsCount];
    check(elementsCount > 0);
    SIZE_T allocationSize = elementsCount * sizeof(T);
    if ((mpAllocIterator + allocationSize) > (mpMemoryPool + mMemoryPoolSize))
    {
        ReallocMemoryPool(mMemoryPoolSize + sReallocStep);
    }
    T* allocated = reinterpret_cast<T*>(mpAllocIterator);
    mpAllocIterator += allocationSize;
    return allocated;
}



/*
    ArenaStateLogger defines and implements the behavior of logging areana state.
*/
class ArenaStateLogger
{

    ArenaStateContext mStartState;
    ArenaStateContext mFinalState;
};