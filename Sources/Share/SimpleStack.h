#pragma once


#ifdef __cplusplus
extern "C" {
#endif


//-----------------------------------------------------------------------------
//  Include files:
//-----------------------------------------------------------------------------
#include <stdint.h>
#include <stdbool.h>

// WARNING: for portable, don't include other header files
#include "Debug.h"

//-----------------------------------------------------------------------------
//  Constant definitions:
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
//  Macros definitions:
//-----------------------------------------------------------------------------

#define SIMPLE_STACK_DEFINE_GEN(TYPE, NAME) \
typedef struct NAME                         \
{                                           \
	unsigned int totalCount;                \
	unsigned int insertIndex;               \
	TYPE *pData;                            \
} NAME##_t;

#define SIMPLE_STACK_IS_FULL_GEN(TYPE, NAME)             \
inline static bool NAME##_IsFull(const NAME##_t *pStack) \
{                                                        \
	return pStack->insertIndex == pStack->totalCount;    \
}

#define SIMPLE_STACK_IS_EMPTY_GEN(TYPE, NAME)             \
inline static bool NAME##_IsEmpty(const NAME##_t *pStack) \
{                                                         \
	return pStack->insertIndex == 0;                      \
}

#define SIMPLE_STACK_POP_GEN(TYPE, NAME)                     \
inline static bool NAME##_Pop(NAME##_t *pStack, TYPE *pData) \
{                                                            \
	if (NAME##_IsEmpty(pStack))                              \
	{                                                        \
		return false;                                        \
	}                                                        \
	pStack->insertIndex--;                                   \
	*pData = *(pStack->pData + pStack->insertIndex);         \
	return true;                                             \
}

#define SIMPLE_STACK_PUSH_GEN(TYPE, NAME)                   \
inline static bool NAME##_Push(NAME##_t *pStack, TYPE data) \
{                                                           \
	if (NAME##_IsFull(pStack))                              \
	{                                                       \
		return false;                                       \
	}                                                       \
	*(pStack->pData + pStack->insertIndex) = data;          \
	pStack->insertIndex++;                                  \
	return true;                                            \
}

#define SIMPLE_STACK_GET_COUNT_GEN(TYPE, NAME)                     \
inline static unsigned int NAME##_GetCount(const NAME##_t *pStack) \
{                                                                  \
	return pStack->insertIndex;                                    \
}

//-----------------------------------------------------------------------------
//  Data type definitions: typedef, struct or class
//-----------------------------------------------------------------------------
typedef struct SimpleStack
{
	unsigned int totalCount;
	unsigned int entrySize;
	unsigned int insertIndex;
	void *pData;
} SimpleStack_t;

//-----------------------------------------------------------------------------
//  Public interface functions:
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
//  Inline functions
//-----------------------------------------------------------------------------
#pragma always_inline
inline static bool SimpleStack_IsFull(const SimpleStack_t *pStack)
{
	return pStack->totalCount == pStack->insertIndex;
}

#pragma always_inline
inline static bool SimpleStack_IsEmpty(const SimpleStack_t *pStack)
{
	return pStack->insertIndex == 0x00;
}

#pragma always_inline
inline static void *SimpleStack_Pop(SimpleStack_t *pStack)
{
	if (SimpleStack_IsEmpty(pStack))
	{
		return NULL;
	}

	pStack->insertIndex --;
	return pStack->pData + pStack->entrySize * pStack->insertIndex;
}

#pragma always_inline
inline static bool SimpleStack_Push(SimpleStack_t *pStack, void *pData)
{
	if (SimpleStack_IsFull(pStack))
	{
		return false;
	}
	memcpy(pStack->pData + pStack->insertIndex * pStack->entrySize, pData, pStack->entrySize);
	pStack->insertIndex ++;
	return true;
}

#pragma always_inline
inline static unsigned int SimpleStack_GetCount(const SimpleStack_t *pStack)
{
	return pStack->insertIndex;
}


#ifdef __cplusplus
}
#endif