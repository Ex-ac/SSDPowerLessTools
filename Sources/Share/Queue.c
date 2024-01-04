



//-----------------------------------------------------------------------------
//  Include files:
//-----------------------------------------------------------------------------
#include "Queue.h"
#include "SimpleFifo.h"

//-----------------------------------------------------------------------------
//  Constants definitions:
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
//  Macros definitions:
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
//  Data type definitions: typedef, struct or class
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
//  Private function proto-type definitions:
//-----------------------------------------------------------------------------
static bool CheckQueueEventHandle(Queue_t *pQueue, QueueEventType_t eventId);
void QueueEventHandle(Queue_t *pQueue, uint_t eventBitmap);

//-----------------------------------------------------------------------------
//  Data declaration: Private or Public
//-----------------------------------------------------------------------------

const QueueInstanceOperator_t cSimpleFifoOperator = {
	.freeCountFunc = (QueueInstanceCountFunc_t)SimpleFifo_FreeCount,
	.countFunc = (QueueInstanceCountFunc_t)SimpleFifo_Count,
	.isEmptyFunc = (QueueInstanceIsEmptyFunc_t)SimpleFifo_IsEmtpy,
	.isFullFunc = (QueueInstanceIsFullFunc_t)SimpleFifo_IsFull,
	.offerFunc = (QueueInstanceOfferFunc_t)SimpleFifo_Offer,
	.peekFunc = (QueueInstancePeekFunc_t)SimpleFifo_Peek,
	.publishFunc = (QueueInstancePublishFunc_t)SimpleFifo_Push,
	.removeFunc = (QueueInstanceRemoveFunc_t)SimpleFifo_Pop,
	.entrySizeFunc = (QueueInstanceEntrySizeFunc_t)SimpleFifo_EntrySize,
};

//-----------------------------------------------------------------------------
//  Imported data proto-type without header include
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
//  Imported function proto-type without header include
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
//  Private functions
//-----------------------------------------------------------------------------

static bool CheckQueueEventHandle(Queue_t *pQueue, QueueEventType_t eventId)
{
	bool run = false;
	switch (eventId)
	{
	case cQueueEventType_OnFull:
		if (Queue_IsFull(pQueue))
		{
			run = true;
		}
		break;

	case cQueueEventType_OnEmpty:
		if (Queue_IsEmpty(pQueue))
		{
			run = true;
		}
		break;

	case cQueueEventType_OnAdd:
	case cQueueEventType_OnRemove:
		run = true;
		break;

	case cQueueEventType_ChangeToNotEmpty:
		if (Queue_Count(pQueue) == 1)
		{
			run = true;
		}
		break;

	default:
		ASSERT(0);
		break;
	}
	return run;
}

void QueueEventHandle(Queue_t *pQueue, uint_t eventBitmap)
{
	QueueEventType_t eventId;
	do
	{
		eventId = Bit_CountTailZero(eventBitmap);
		if (CheckQueueEventHandle(pQueue, eventId))
		{
			pQueue->event.handler[eventId](pQueue);
		}
		eventBitmap &= (~BIT(eventId));
	} while (eventBitmap != 0x00);
}
//-----------------------------------------------------------------------------
//  Public functions
//-----------------------------------------------------------------------------
void Queue_RegisterEventHandler(Queue_t *pQueue, QueueEventType_t eventId, QueueEventHandler_t handler)
{
	ASSERT_DEBUG(eventId < cNumberOfQueueEventType);
	if (pQueue->event.activeEventBitmap & eventId)
	{
		// report
	}
	pQueue->event.activeEventBitmap |= BIT(eventId);
	pQueue->event.handler[eventId] = handler;
}

void Queue_Init(Queue_t *pQueue, void *pInstance, const QueueInstanceOperator_t *pInstanceOpc)
{
	memset(pQueue, 0, sizeof(Queue_t));
	pQueue->pInstance = pInstance;
	pQueue->pInstanceOperator = pInstanceOpc;
	DebugPrint("%p/%p/%p", pQueue, pQueue->pInstance, pQueue->pInstanceOperator);
}

Queue_t *Queue_Create(uint_t entryNum, uint_t entrySize)
{
	Queue_t *pQueue = (Queue_t *)malloc(sizeof(Queue_t));
	if (pQueue != NULL)
	{
		SimpleFifo_t *pFifo = SimpleFifo_Create(entryNum, entrySize);
		if (pFifo != NULL)
		{
			Queue_Init(pQueue, pFifo, &cSimpleFifoOperator);
		}
		else
		{
			free(pQueue);
			pQueue = NULL;
		}
	}
	return pQueue;

}

void Queue_Destroy(Queue_t *pQueue)
{
	SimpleFifo_Destroy(pQueue->pInstance);
	free(pQueue);
}