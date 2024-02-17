#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

//-----------------------------------------------------------------------------
//  Include files:
//-----------------------------------------------------------------------------
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "Bit.h"
#include "Global.h"
#include "Debug.h"

//-----------------------------------------------------------------------------
//  Constant definitions:
//-----------------------------------------------------------------------------
typedef enum QueueEventType
{
	cQueueEventType_OnFull = 0x00,
	cQueueEventType_OnEmpty,
	cQueueEventType_OnAdd,
	cQueueEventType_OnRemove,
	cQueueEventType_ChangeToNotEmpty,
	cNumberOfQueueEventType,
} QueueEventType_t;

typedef enum QueueEventBitmap
{
	cQueueEventBitmapOnRemove = (BIT(cQueueEventType_OnEmpty) | BIT(cQueueEventType_OnRemove)),
	cQueueEventBitmapOnPublish = (BIT(cQueueEventType_OnFull) | BIT(cQueueEventType_OnAdd) | BIT(cQueueEventType_ChangeToNotEmpty)),
} QueueEventBitmap_t;
//-----------------------------------------------------------------------------
//  Macros definitions:
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
//  Data type definitions: typedef, struct or class
//-----------------------------------------------------------------------------
struct Queue;

typedef uint_t (*QueueInstanceCountFunc_t)(void *pInstance);
typedef bool (*QueueInstanceIsEmptyFunc_t)(void *pInstance);
typedef bool (*QueueInstanceIsFullFunc_t)(void *pInstance);
typedef void *(*QueueInstancePeekFunc_t)(void *pInstance);
typedef void (*QueueInstanceRemoveFunc_t)(void *pInstance);
typedef void *(*QueueInstanceOfferFunc_t)(void *pInstance);
typedef void (*QueueInstancePublishFunc_t)(void *pInstance);
typedef uint_t (*QueueInstanceEntrySizeFunc_t)(void *pInstance);
typedef void (*QueueEventHandler_t)(struct Queue *pQueue);

typedef struct QueueEvent
{
	uint_t activeEventBitmap;
	QueueEventHandler_t handler[cNumberOfQueueEventType];
} QueueEvent_t;

typedef struct QueueInstanceOperator
{
	QueueInstanceCountFunc_t freeCountFunc;
	QueueInstanceCountFunc_t countFunc;
	QueueInstanceIsEmptyFunc_t isEmptyFunc;
	QueueInstanceIsFullFunc_t isFullFunc;
	QueueInstancePeekFunc_t peekFunc;
	QueueInstanceRemoveFunc_t removeFunc;
	QueueInstanceOfferFunc_t offerFunc;
	QueueInstancePublishFunc_t publishFunc;
	QueueInstanceEntrySizeFunc_t entrySizeFunc;
} QueueInstanceOperator_t;

typedef struct Queue
{
	void *pInstance;
	const QueueInstanceOperator_t *pInstanceOperator;
	QueueEvent_t event;
	uint_t param;
} Queue_t;


extern const QueueInstanceOperator_t cSimpleFifoOperator;
//-----------------------------------------------------------------------------
//  Public interface functions:
//-----------------------------------------------------------------------------

void Queue_RegisterEventHandler(Queue_t *pQueue, QueueEventType_t eventId, QueueEventHandler_t handler);
void Queue_Init(Queue_t *pQueue, void *pInstance, const QueueInstanceOperator_t *pInstanceOpc);
Queue_t *Queue_Create(uint_t entryNum, uint_t entrySize);
void Queue_Destroy(Queue_t *pQueue);
//-----------------------------------------------------------------------------
//  Inline functions
//-----------------------------------------------------------------------------

#pragma always_inline
inline static uint_t Queue_Count(const Queue_t *pQueue)
{
	return pQueue->pInstanceOperator->countFunc(pQueue->pInstance);
}

#pragma always_inline
inline static uint_t Queue_FreeCount(const Queue_t *pQueue)
{
	return pQueue->pInstanceOperator->freeCountFunc(pQueue->pInstance);
}

#pragma always_inline
inline static bool Queue_IsFull(const Queue_t *pQueue)
{
	return pQueue->pInstanceOperator->isFullFunc(pQueue->pInstance);
}

#pragma always_inline
inline static bool Queue_IsEmpty(const Queue_t *pQueue)
{
	DebugPrint("%p/%p/%p/%p", pQueue, pQueue->pInstance, pQueue->pInstanceOperator->isEmptyFunc, pQueue->pInstance);
	bool ret =  pQueue->pInstanceOperator->isEmptyFunc(pQueue->pInstance);
	DebugPrint("return %d", ret);
	return ret;
}

#pragma always_inline
inline static void *Queue_Peek(const Queue_t *pQueue)
{
	return pQueue->pInstanceOperator->peekFunc(pQueue->pInstance);
}

#pragma always_inline
inline static void Queue_Remove(Queue_t *pQueue)
{
	extern void QueueEventHandle(Queue_t * pQueue, uint_t eventId);

	pQueue->pInstanceOperator->removeFunc(pQueue->pInstance);
	uint_t eventBitmap = pQueue->event.activeEventBitmap & cQueueEventBitmapOnRemove;

	if (eventBitmap != 0x00)
	{
		QueueEventHandle(pQueue, eventBitmap);
	}
}

#pragma always_inline
inline static void *Queue_Offer(const Queue_t *pQueue)
{
	return pQueue->pInstanceOperator->offerFunc(pQueue->pInstance);
}

#pragma always_inline
inline static void Queue_Publish(Queue_t *pQueue)
{
	extern void QueueEventHandle(Queue_t * pQueue, uint_t eventBitmap);

	DebugPrint("start");
	pQueue->pInstanceOperator->publishFunc(pQueue->pInstance);
	uint_t eventBitmap = pQueue->event.activeEventBitmap & cQueueEventBitmapOnPublish;

	if (eventBitmap != 0x00)
	{
		QueueEventHandle(pQueue, eventBitmap);
	}
}

#pragma always_inline
inline static uint_t Queue_EntrySize(const Queue_t *pQueue)
{
	return pQueue->pInstanceOperator->entrySizeFunc(pQueue->pInstance);
}

#pragma always_inline
inline static void *Queue_Pop(Queue_t *pQueue)
{
	void *pEntry = Queue_Peek(pQueue);

	if (pEntry != NULL)
	{
		Queue_Remove(pQueue);
	}
	return pEntry;
}

#pragma always_inline
inline static bool Queue_Push(Queue_t *pQueue, void *pEntry)
{
	void *pOfferEntry = Queue_Offer(pQueue);
	if (pOfferEntry == NULL)
	{
		return false;
	}
	memcpy(pOfferEntry, pEntry, Queue_EntrySize(pQueue));
	Queue_Publish(pQueue);
	return true;
}

#ifdef __cplusplus
}
#endif