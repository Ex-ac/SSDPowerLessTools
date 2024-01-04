#pragma once


#ifdef __cplusplus
extern "C" {
#endif


//-----------------------------------------------------------------------------
//  Include files:
//-----------------------------------------------------------------------------
#include <stdint.h>
#include <stdbool.h>
#include "Type.h"
#include "Debug.h"

//-----------------------------------------------------------------------------
//  Constant definitions:
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
//  Macros definitions:
//-----------------------------------------------------------------------------

#define SIMPLE_STACK_DEFINE_GEN(TYPE) \
typedef struct SimpleStack##TYPE      \
{                                     \
	uint_t totalCount;                \
	uint_t insertIndex;               \
	TYPE *pData;                      \
} SimpleStack##TYPE##_t;

#define SIMPLE_STACK_IS_FULL_GEN(TYPE)                                             \
inline static bool SimpleStack##TYPE##_IsFull(const SimpleStack##TYPE##_t *pStack) \
{                                                                                  \
	return pStack->insertIndex == pStack->totalCount;                              \
}

#define SIMPLE_STACK_IS_EMPTY_GEN(TYPE)                                             \
inline static bool SimpleStack##TYPE##_IsEmpty(const SimpleStack##TYPE##_t *pStack) \
{                                                                                   \
	return pStack->insertIndex == 0;                                                \
}

#define SIMPLE_STACK_POP_GEN(TYPE)                                                     \
inline static bool SimpleStack##TYPE##_Pop(SimpleStack##TYPE##_t *pStack, TYPE *pData) \
{                                                                                      \
	if (SimpleStack##TYPE##_IsEmpty(pStack))                                           \
	{                                                                                  \
		return false;                                                                  \
	}                                                                                  \
	pStack->insertIndex--;                                                             \
	*pData = *(pStack->pData + pStack->insertIndex);                                   \
	return true;                                                                       \
}

#define SIMPLE_STACK_PUSH_GEN(TYPE)                                                   \
inline static bool SimpleStack##TYPE##_Push(SimpleStack##TYPE##_t *pStack, TYPE data) \
{                                                                                     \
	if (SimpleStack##TYPE##_IsFull(pStack))                                           \
	{                                                                                 \
		return false;                                                                 \
	}                                                                                 \
	*(pStack->pData + pStack->insertIndex) = data;                                    \
	pStack->insertIndex++;                                                            \
	return true;                                                                      \
}

//-----------------------------------------------------------------------------
//  Data type definitions: typedef, struct or class
//-----------------------------------------------------------------------------
typedef struct SimpleStack
{
	uint_t totalCount;
	uint_t entrySize;
	uint_t insertIndex;
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


#ifdef __cplusplus
}
#endif