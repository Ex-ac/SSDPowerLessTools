#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

//-----------------------------------------------------------------------------
//  Include files:
//-----------------------------------------------------------------------------
#include <stdint.h>
#include "Type.h"
#include "Debug.h"

//-----------------------------------------------------------------------------
//  Constant definitions:
//-----------------------------------------------------------------------------



//-----------------------------------------------------------------------------
//  Macros definitions:
//-----------------------------------------------------------------------------

#define SIMPLE_FIFO_TYPE_DEFINE_GEN(TYPE) \
typedef struct SimpleFifo##TYPE           \
{                                         \
	volatile uint_t frontIndex;           \
	volatile uint_t rearIndex;            \
	uint_t mask;                          \
	TYPE *pEntry;                         \
} SimpleFifo##TYPE##_t;

#define SIMPLE_FIFO_TYPE_INIT_GEN(TYPE)                                                                           \
inline static void SimpleFifo##TYPE##_Init(SimpleFifo##TYPE##_t *pFifo, uint32_t totalCount, TYPE *entryBaseAddr) \
{                                                                                                                 \
	pFifo->frontIndex = pFifo->rearIndex = 0;                                                                     \
	pFifo->mask = totalCount - 1;                                                                                 \
	pFifo->pEntry = entryBaseAddr;                                                                                \
}

#define SIMPLE_FIFO_TYPE_COUNT_GEN(TYPE)                                             \
inline static uint_t SimpleFifo##TYPE##_Count(const SimpleFifo##TYPE##_t *pFifo)     \
{                                                                                    \
	return (pFifo->mask + 1 + pFifo->rearIndex - pFifo->frontIndex) & (pFifo->mask); \
}

#define SIMPLE_FIFO_TYPE_IS_FULL_GEN(TYPE)                                      \
inline static bool SimpleFifo##TYPE##_IsFull(const SimpleFifo##TYPE##_t *pFifo) \
{                                                                               \
	return pFifo->frontIndex == ((pFifo->rearIndex + 1) & pFifo->mask);         \
}

#define SIMPLE_FIFO_TYPE_IS_EMPTY_GEN(TYPE)                                      \
inline static bool SimpleFifo##TYPE##_IsEmpty(const SimpleFifo##TYPE##_t *pFifo) \
{                                                                                \
	return pFifo->frontIndex == pFifo->rearIndex;                                \
}

#define SIMPLE_FIFO_TYPE_POP_GEN(TYPE)                                              \
inline static bool SimpleFifo##TYPE##_Pop(SimpleFifo##TYPE##_t *pFifo, TYPE *pData) \
{                                                                                   \
	if (SimpleFifo##TYPE##_IsEmpty(pFifo))                                          \
	{                                                                               \
		return false;                                                               \
	}                                                                               \
	*pData = *(pFifo->pEntry + pFifo->frontIndex);                                  \
	pFifo->frontIndex = (pFifo->frontIndex + 1) & pFifo->mask;                      \
	return true;                                                                    \
}

#define SIMPLE_FIFO_TYPE_PUSH_GEN(TYPE)                                                  \
inline static bool SimpleFifo##TYPE##_Push(SimpleFifo##TYPE##_t *pFifo, const TYPE data) \
{                                                                                        \
	if (SimpleFifo##TYPE##_IsFull(pFifo))                                                \
	{                                                                                    \
		return false;                                                                    \
	}                                                                                    \
	*(pFifo->pEntry + pFifo->rearIndex) = data;                                          \
	pFifo->rearIndex = (pFifo->rearIndex + 1) & pFifo->mask;                             \
	return true;                                                                         \
}

//-----------------------------------------------------------------------------
//  Data type definitions: typedef, struct or class
//-----------------------------------------------------------------------------
typedef struct SimpleFifo
{
	volatile uint_t frontIndex;
	volatile uint_t rearIndex;
	uint_t mask;
	uint_t entrySize;
	void *pEntry;
} SimpleFifo_t;

//-----------------------------------------------------------------------------
//  Public interface functions:
//-----------------------------------------------------------------------------
void SimpleFifo_Init(SimpleFifo_t *pFifo, uint_t entryNum, uint_t entrySize, void *pEntry);
SimpleFifo_t *SimpleFifo_Create(uint_t entryNum, uint_t entrySize);
void SimpleFifo_Destroy(SimpleFifo_t *pFifo);

//-----------------------------------------------------------------------------
//  Inline functions
//-----------------------------------------------------------------------------

#pragma always_inline
inline static bool SimpleFifo_IsEmtpy(const SimpleFifo_t *pFifo)
{
	// DebugPrint("%p/%p", pFifo, __builtin_return_address(0));
	return pFifo->frontIndex == pFifo->rearIndex;
}

#pragma always_inline
inline static bool SimpleFifo_IsFull(const SimpleFifo_t *pFifo)
{
	// DebugPrint("");
	return pFifo->frontIndex == ((pFifo->rearIndex + 1) & pFifo->mask);
}

#pragma always_inline
inline static uint_t SimpleFifo_Count(const SimpleFifo_t *pFifo)
{
	return (pFifo->mask + 1 + pFifo->rearIndex - pFifo->frontIndex) & (pFifo->mask);
}

#pragma always_inline
inline static uint_t SimpleFifo_FreeCount(const SimpleFifo_t *pFifo)
{
	return pFifo->mask - SimpleFifo_Count(pFifo);
}

#pragma always_inline
inline static uint_t SimpleFifo_EntrySize(const SimpleFifo_t *pFifo)
{
	return pFifo->entrySize;
}

#pragma always_inline
inline static void SimpleFifo_Push(SimpleFifo_t *pFifo)
{
	ASSERT_DEBUG(!SimpleFifo_IsFull(pFifo));
	pFifo->rearIndex = (pFifo->rearIndex + 1) & pFifo->mask;
}

#pragma always_inline
inline static void SimpleFifo_Pop(SimpleFifo_t *pFifo)
{
	ASSERT_DEBUG(!SimpleFifo_IsEmtpy(pFifo));
	pFifo->frontIndex = pFifo->frontIndex + 1 & pFifo->mask;
}

#pragma always_inline
inline static void* SimpleFifo_Offer(SimpleFifo_t *pFifo)
{
	return SimpleFifo_IsFull(pFifo) ? NULL : pFifo->pEntry + pFifo->rearIndex * pFifo->entrySize;
}

#pragma always_inline
inline static void* SimpleFifo_Peek(SimpleFifo_t *pFifo)
{
	return SimpleFifo_IsEmtpy(pFifo) ? NULL : pFifo->pEntry + pFifo->frontIndex * pFifo->entrySize;
}


#ifdef __cplusplus
}
#endif