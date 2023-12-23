#pragma once

#include <stdint.h>
#include "Type.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct SimpleFifo
{
	uint_t frontIndex;
	uint_t rearIndex;
	uint_t mask;
	uint_t entrySize;
	void *pEntry;
} SimpleFifo_t;

void SimpleFifo_Init(SimpleFifo_t *pFifo, uint_t entryNum, uint_t entrySize, void *pEntry);

#pragma always_inline
bool SimpleFiFo_IsEmtpy(const SimpleFifo_t *pFifo)
{
	return pFifo->frontIndex == pFifo->rearIndex;
}

bool SimpleFifo_IsFull(const SimpleFifo_t *pFifo)
{

}

#pragma always_inline
uint_t SimpleFifo_Size(const SimpleFifo_t *pFifo)
{
	return (pFifo->mask + 1 + pFifo->rearIndex - pFifo->frontIndex) & (pFifo->mask);
}


#pragma always_inline
void SimpleFifo_InQueue(SimpleFifo_t *pFifo)
{
	ASSERT_DEBUG(!SimpleFifo_IsFull(pFifo));
	pFifo->rearIndex = (pFifo->rearIndex + 1) & pFifo->mask;
}

void SimpleFifo_DeQueue(SimpleFifo_t *pFifo)
{
	ASSERT_DEBUG(!SimpleFiFo_IsEmtpy(pFifo));
	pFifo->frontIndex = pFifo->frontIndex + 1 & pFifo->mask;
}

#define SIMPLE_FIFO_TYPE_DEFINE_GEN(TYPE) \
typedef struct SimpleFifo##TYPE           \
{                                         \
	uint32_t inIndex;                     \
	uint32_t outIndex;                    \
	uint32_t mask;                        \
	TYPE *entryBaseAddr;                  \
} SimpleFifo##TYPE##_t;

#define SIMPLE_FIFO_TYPE_INIT_GEN(TYPE)                                                                                  \
inline static void SimpleFifo##TYPE##_Init(SimpleFifo##TYPE##_t *fifo, uint32_t totalCount, TYPE *entryBaseAddr) \
{                                                                                                                        \
	fifo->inIndex = fifo->outIndex = 0;                                                                                  \
	fifo->mask = totalCount - 1;                                                                                         \
	fifo->entryBaseAddr = entryBaseAddr;                                                                                 \
}

#define SIMPLE_FIFO_TYPE_COUNT_GEN(TYPE)                              \
inline static uint_t SimpleFifo##TYPE##_Count(const SimpleFifo##TYPE##_t *fifo) \
{                                                                     \
	return (fifo->mask + 1) - (fifo->inIndex - fifo->outIndex);       \
}

#define SIMPLE_FIFO_TYPE_IS_FULL_GEN(TYPE)                                             \
inline static bool SimpleFifo##TYPE##_IsFull(const SimpleFifo##TYPE##_t *fifo) \
{                                                                                      \
	return SimpleFifo##TYPE##_Count(fifo) == fifo->mask;                               \
}

#define SIMPLE_FIFO_TYPE_IS_EMPTY_GEN(TYPE)                                             \
inline static bool SimpleFifo##TYPE##_IsEmpty(const SimpleFifo##TYPE##_t *fifo) \
{                                                                                       \
	return SimpleFifo##TYPE##_Count(fifo) == 0;                                         \
}

#define SIMPLE_FIFO_TYPE_GET_GEN(TYPE)                                                     \
inline static bool SimpleFifo##TYPE##_Get(SimpleFifo##TYPE##_t *fifo, TYPE *pData) \
{                                                                                          \
	if (SimpleFifo##TYPE##_IsEmpty(fifo))                                                  \
	{                                                                                      \
		return false;                                                                      \
	}                                                                                      \
	*pData = *(fifo->entryBaseAddr + fifo->outIndex);                                      \
	fifo->outIndex = (fifo->outIndex + 1) & fifo->mask;                                    \
	return true;                                                                           \
}

#define SIMPLE_FIFO_TYPE_PUT_GEN(TYPE)                                                   \
inline static bool SimpleFifo##TYPE##_Put(SimpleFifo##TYPE##_t *fifo, TYPE data) \
{                                                                                        \
	if (SimpleFifo##TYPE##_IsFull(fifo))                                                 \
	{                                                                                    \
		return false;                                                                    \
	}                                                                                    \
	*(fifo->entryBaseAddr + fifo->inIndex) = data;                                       \
	fifo->inIndex = (fifo->inIndex + 1) & fifo->mask;                                    \
	return true;                                                                         \
}

#ifdef __cplusplus
}
#endif