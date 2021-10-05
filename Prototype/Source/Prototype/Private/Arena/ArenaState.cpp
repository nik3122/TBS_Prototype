
#include "Arena/ArenaState.h"
#include "Arena/ArenaUtilities.h"
#include "UnknownCommon.h"


ArenaStateContext::ArenaStateContext()
    : mMemoryPoolSize(0)
    , mpMemoryPool(nullptr)
    , mpAllocIterator(nullptr)
{
    ReallocMemoryPool(sMemoryPoolSize);
    Reset();
}

ArenaStateContext::~ArenaStateContext()
{
    if (mpMemoryPool != nullptr)
    {
        FMemory::Free(mpMemoryPool);
    }
}

void ArenaStateContext::Reset()
{
    mpAllocIterator = mpMemoryPool;
}

void ArenaStateContext::ReallocMemoryPool(SIZE_T newMemoryPoolSize)
{
    check(newMemoryPoolSize > mMemoryPoolSize);
    byte* pNewMemoryPool = reinterpret_cast<byte*>(FMemory::Malloc(newMemoryPoolSize, sMemoryAligment));
    FMemory::Memcpy(pNewMemoryPool, mpMemoryPool, mMemoryPoolSize);
    mpAllocIterator = (mpAllocIterator - mpMemoryPool) + pNewMemoryPool;
    mpMemoryPool = pNewMemoryPool;
    mMemoryPoolSize = newMemoryPoolSize;
    UE_LOG(ArenaCommonLog, Warning, TEXT("Arena state memory pool reallocation (%i bytes)."), mMemoryPoolSize);
}
