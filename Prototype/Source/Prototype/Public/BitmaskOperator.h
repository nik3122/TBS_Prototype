#pragma once


#include "CoreMinimal.h"


template <class T>
int32 MakeBitMask(T flag)
{
    return 1u << static_cast<uint32>(flag);
}

template <class T>
int32 MakeBitMask(T flag0, T flag1)
{
    return (1u << static_cast<uint32>(flag0)) | MakeBitMask(flag1);
}

template <class T>
int32 MakeBitMask(T flag0, T flag1, T flag2)
{
    return (1u << static_cast<uint32>(flag0)) | MakeBitMask(flag1, flag2);
}

template <class T>
int32 MakeBitMask(T flag0, T flag1, T flag2, T flag3)
{
    return (1u << static_cast<uint32>(flag0)) | MakeBitMask(flag1, flag2, flag3);
}

template <class T>
int32 MakeBitMask(T flag0, T flag1, T flag2, T flag3, T flag4)
{
    return (1u << static_cast<uint32>(flag0)) | MakeBitMask(flag1, flag2, flag3, flag4);
}

template <class T>
int32 MakeBitMask(T flag0, T flag1, T flag2, T flag3, T flag4, T flag5)
{
    return (1u << static_cast<uint32>(flag0)) | MakeBitMask(flag1, flag2, flag3, flag4, flag5);
}

template <class T>
static bool IsFlagAtBitMask(int32 mask, T flag)
{
    return mask & MakeBitMask(flag);
}