/**
 * @file IoEngine.c
 * @author Exac (ex-ac@outlook.com)
 * @brief
 * @version 0.1
 * @date 2024-01-03
 *
 * @copyright Copyright (c) 2024
 *
 */


//-----------------------------------------------------------------------------
//  Include files:
//-----------------------------------------------------------------------------
#define _GNU_SOURCE
#define __STDC_FORMAT_MACROS

#include "IoEngine.h"
#include <pthread.h>
#include <semaphore.h>
#include <memory.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <sys/epoll.h>
#include <time.h>

#include "SimpleFifo.h"
#include "SimpleList.h"
#include "Debug.h"
#include "Queue.h"
#include "CommonCommandPool.h"


//-----------------------------------------------------------------------------
//  Constants definitions:
//-----------------------------------------------------------------------------
/**
 * @brief request thread context create statue
 * 
 */
typedef enum RequestThreadStatue
{
	cRequestThreadStatue_Instance = 0x00,	// alloc the instance
	cRequestThreadStatue_Semaphore,			// init the semaphore
	cRequestThreadStatue_Queue,				// init the queue
	cRequestThreadStatue_Thread,			// create the thread
	cRequestThreadStatue_Completed,			//	create completed
} RequestThreadStatue_t;

typedef enum CompletedThreadStatue
{
	cCompletedThreadStatue_Instance = 0x00,		// alloc the instance
	cCompletedThreadStatue_WaitCommandItemList, // init the wait item list
	cCompletedThreadStatue_WaitQueue,			// init the wait queue
	cCompletedThreadStatue_CompletedQueue,		// init the  completed queue
	cCompletedThreadStatue_Epoll,				// init the epoll
	cCompletedThreadStatue_Thread,				// create the thread
	cCompletedThreadStatue_Completed,			//	create completed
} CompletedThreadStatue_t;

//-----------------------------------------------------------------------------
//  Macros definitions:
//-----------------------------------------------------------------------------

#ifdef Uint
#undef Uint
#endif
// using the unsigned int to save addr
#define Uint			uint_t



//-----------------------------------------------------------------------------
//  Data type definitions: typedef, struct or class
//-----------------------------------------------------------------------------

/**
 * @brief io command request queue process thread context
 * 
 */
typedef struct RequestThreadContext
{
	pthread_mutex_t mutex;
	pthread_cond_t semaphore;
	CommandIdFifo_t requestQueue;	// to save the usr request
	pthread_t thread;
	bool abort				: 1;	// return all the requests to completed queue
	bool exit				: 1;	// do abort and exit the thread
	bool ready			: 1;	// the thread is ready to process the request		
	IoEngine_t *owner;
	unsigned int processorCount;
} RequestThreadContext_t;


typedef struct WaitCommandItem
{
	CommandId_t commandId;
	struct WaitCommandItem *pNext;
} WaitCommandItem_t;

/**
 * @brief io command completed queue process thread context
 * 
 */
typedef struct CompletedThreadContext
{
	SimpleList_t waitCommandItemFreeList;
	SimpleList_t waitCommandItemList;
	WaitCommandItem_t *pWaitCommandItemInstance;
	CommandIdFifo_t completedQueue;		// to save the complete from driver
	CommandIdFifo_t waitQueue;			// wait for complete
	pthread_t thread;
	struct epoll_event epollEvent;
	int epollFileHandler;
	bool exit				: 1;	// when completed queue empty, exit the thread
	bool ready				: 1;
	IoEngine_t *owner;
	unsigned int waitCount;
	unsigned int completedCount;
} CompletedThreadContext_t;







//-----------------------------------------------------------------------------
//  Private function proto-type definitions:
//-----------------------------------------------------------------------------
/**
 * @brief create the request thread context
 * 
 * @param queueDepth the request queue depth
 * @param owner the io engine owner
 * @return RequestThreadContext_t* 
 */
static RequestThreadContext_t *RequestThreadContext_Creat(uint_t queueDepth, IoEngine_t *owner);

/**
 * @brief destroy the request thread context by statue
 * 
 * @param pContext 
 * @param statue create statue
 */
static void RequestThread_Destroy(RequestThreadContext_t *pContext, int statue);

/**
 * @brief request thread processor function
 * 
 * @param param pointer to RequestThreadContext_t instance
 * @return unless
 */
static void *RequestThread_Processor(void *param);

/**
 * @brief exit request thread 
 * 
 * @param pContext 
 */
static void RequestThread_Exit(RequestThreadContext_t *pContext);

/**
 * @brief abort all the request in the request queue
 * 
 * @param pContext 
 */
static void RequestThread_Abort(RequestThreadContext_t *pContext);

/**
 * @brief notify the request thread wake up
 * 
 * @param pContext
 */
static void RequestThread_Notify(RequestThreadContext_t *pContext);


static CompletedThreadContext_t *CompletedThreadContext_Creat(uint_t queueDepth, IoEngine_t *owner);

static void CompletedThreadContext_Destroy(CompletedThreadContext_t *pContext, int statue);

static void CompletedThread_Exit(CompletedThreadContext_t *pContext);

static void *CompletedThread_Processor(void *param);

static void *WaitCommandItemGetNext(const WaitCommandItem_t *pItem);
static void WaitCommandItemSetNext(WaitCommandItem_t *pItem, void *pNext);

static bool PushToWaitQueue(CompletedThreadContext_t *pContext, CommandId_t commandId);

static void CompletedThread_Exit(CompletedThreadContext_t *pContext);

//-----------------------------------------------------------------------------
//  Data declaration: Private or Public
//-----------------------------------------------------------------------------

const static SimpleListEntryOperator_t cWaitCommandItemEntryOperator =
{
	.getNextFunc = (SimpleListGetNextFunc_t)WaitCommandItemGetNext,
	.setNextFunc = (SimpleListSetNextFunc_t)WaitCommandItemSetNext,
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

static void *RequestThread_Processor(void *param)
{
	RequestThreadContext_t *pContext = (RequestThreadContext_t *)param;
	int count = 0;
	DebugPrint("start");

	pContext->abort = false;
	pContext->ready = true;

	CommandId_t commandId;
	while (true)
	{
		// DebugPrint("wait semaphore");
		pthread_mutex_lock(&pContext->mutex);
		if (CommandIdFifo_IsEmpty(&pContext->requestQueue))
		{
			// DebugPrint("wait semaphore");
			pthread_cond_wait(&pContext->semaphore, &pContext->mutex);
		}
		else
		{
			// __breakpoint();
			DebugPrint("no wait semaphore");
		}
		pthread_mutex_unlock(&pContext->mutex);
		// DebugPrint("get semaphore");
		
		
		while ((count != 0) || (count = CommandIdFifo_Count(&pContext->requestQueue)) != 0)
		{
			bool ok = CommandIdFifo_Pop(&pContext->requestQueue, &commandId);
			ASSERT_DEBUG(ok);

			CommonCommand_t *pCommand = CommonCommandPool_GetCommand(commandId);
			DebugPrint("process request. command id: %d/%d", commandId, pCommand->config.commandId);
			pCommand->time.submitTime = time(NULL);
			if (pContext->exit || pContext->abort)
			{
				pCommand->config.status = cCommandStatus_Abort;
			}
			else
			{
				// do the request, and 
				pCommand->config.status = cCommandStatus_Submit;

			}
			// move to wait queue
			pContext->processorCount ++;
			while (PushToWaitQueue(pContext->owner->completedThreadContext, commandId) == false)
			{
				// wait for completed queue free
				usleep(10);
			}
			

			// usleep(10000);
			count --;
		}
		// DebugPrint("queue empty");

		if (pContext->exit)
		{
			break;
		}
	}
	DebugPrint("exit");
	pthread_exit(NULL);
	return NULL;
}

static RequestThreadContext_t *RequestThreadContext_Creat(uint_t queueDepth, IoEngine_t *owner)
{
	RequestThreadStatue_t status = cRequestThreadStatue_Instance;
	RequestThreadContext_t *pContext = malloc(sizeof(RequestThreadContext_t));
	if (pContext == NULL)
	{
		goto Failed;
	}
	memset(pContext, 0, sizeof(RequestThreadContext_t));

	pContext->owner = owner;
	DebugPrint("Context malloc %p/%p", pContext, &pContext->requestQueue);
	status ++;
	pContext->abort = true;

	
	pthread_mutex_init(&pContext->mutex, NULL);
	pthread_cond_init(&pContext->semaphore, NULL);

	status ++;

	// create queue
	void *pEntryAddr = malloc(sizeof(uint_t) * queueDepth);

	if (pEntryAddr == NULL)
	{
		goto Failed;
	}

	CommandIdFifo_Init(&pContext->requestQueue, queueDepth, pEntryAddr);
	status ++;

	pthread_create(&pContext->thread, NULL, RequestThread_Processor, pContext);
	status ++;

	// pContext->abort = false;

	return pContext;

Failed:
	RequestThread_Destroy(pContext, status);
	return NULL;
}


static void RequestThread_Destroy(RequestThreadContext_t *pContext, int statue)
{
	DebugPrint("start %d", statue);
	if (pContext == NULL)
	{
		return;
	}

	switch (statue)
	{
		case cRequestThreadStatue_Completed:
			RequestThread_Exit(pContext);
			DebugPrint("wait thread exit");
			pthread_join(pContext->thread, NULL);
			DebugPrint("thread exit");
			// free(pContext->thread);
			// pContext->thread = NULL;
		
		case cRequestThreadStatue_Thread:
			// free(pContext->requestQueue);
			free(pContext->requestQueue.pEntry);

		case cRequestThreadStatue_Queue:
			//sem_destroy(&pContext->semaphore);
			pthread_mutex_destroy(&pContext->mutex);
			pthread_cond_destroy(&pContext->semaphore);
		
		case cRequestThreadStatue_Semaphore:
			free(pContext);

		case cRequestThreadStatue_Instance:

			break;
		default:
			ASSERT(0);
			break;
	}

}


static void RequestThread_Exit(RequestThreadContext_t *pContext)
{
	DebugPrint("start");
	pContext->exit = true;
	RequestThread_Notify(pContext);
	// abort all the request
}

static void RequestThread_Abort(RequestThreadContext_t *pContext)
{
	DebugPrint("start");
	pContext->abort = true;
	
	// abort all the request
}

static void RequestThread_Notify(RequestThreadContext_t *pContext)
{
	DebugPrint("trigger");
	pthread_mutex_lock(&pContext->mutex);
	pthread_cond_signal(&pContext->semaphore);
	pthread_mutex_unlock(&pContext->mutex);
}




static CompletedThreadContext_t *CompletedThreadContext_Creat(uint_t queueDepth, IoEngine_t *owner)
{
	queueDepth *= 2;
	CompletedThreadStatue_t status = cCompletedThreadStatue_Instance;
	CompletedThreadContext_t *pContext = malloc(sizeof(CompletedThreadContext_t));
	if (pContext == NULL)
	{
		goto Failed;
	}
	memset(pContext, 0, sizeof(CompletedThreadContext_t));

	pContext->owner = owner;
	status ++;

	
	
	// create wait command item instance
	pContext->pWaitCommandItemInstance = malloc(sizeof(WaitCommandItem_t) * queueDepth);
	DebugPrint("Context malloc %p/%p %ld/%d", pContext, pContext->pWaitCommandItemInstance, sizeof(WaitCommandItem_t) * queueDepth, queueDepth);

	if (pContext->pWaitCommandItemInstance == NULL)
	{
		goto Failed;
	}


	// create wait command item list
	SimpleList_Init(&pContext->waitCommandItemList, &cWaitCommandItemEntryOperator);
	SimpleList_Init(&pContext->waitCommandItemFreeList, &cWaitCommandItemEntryOperator);

	// move to free list
	for (int i = 0; i < queueDepth; i++)
	{
		SimpleList_PushToTail(&pContext->waitCommandItemFreeList, &pContext->pWaitCommandItemInstance[i], &pContext->pWaitCommandItemInstance[i], 1);
	}
	status ++;

	// create wait queue
	void *pEntryAddr = malloc(sizeof(uint_t) * queueDepth);

	if (pEntryAddr == NULL)
	{
		goto Failed;
	}

	CommandIdFifo_Init(&pContext->waitQueue, queueDepth, pEntryAddr);
	status ++;


	// create completed queue
	pEntryAddr = malloc(sizeof(uint_t) * queueDepth);

	if (pEntryAddr == NULL)
	{
		goto Failed;
	}

	CommandIdFifo_Init(&pContext->completedQueue, queueDepth, pEntryAddr);
	status ++;



	// create epoll
	pContext->epollFileHandler = epoll_create1(0);
	if (pContext->epollFileHandler == -1)
	{
		goto Failed;
	}
	status ++;

	// create thread
	pthread_create(&pContext->thread, NULL, CompletedThread_Processor, pContext);
	status ++;

	return pContext;
Failed:
	CompletedThreadContext_Destroy(pContext, status);
	return NULL;
}

static void CompletedThreadContext_Destroy(CompletedThreadContext_t *pContext, int statue)
{
	DebugPrint("start %d", statue);
	if (pContext == NULL)
	{
		return;
	}

	switch (statue)
	{
		case cCompletedThreadStatue_Completed:
			CompletedThread_Exit(pContext);
			DebugPrint("wait thread exit");
			pthread_join(pContext->thread, NULL);
			DebugPrint("thread exit");
			// free(pContext->thread);
			// pContext->thread = NULL;
		
		case cCompletedThreadStatue_Thread:
			// free(pContext->requestQueue);
			close(pContext->epollFileHandler);

		case cCompletedThreadStatue_Epoll:
			free(pContext->completedQueue.pEntry);

		case cCompletedThreadStatue_CompletedQueue:
			free(pContext->waitQueue.pEntry);
		
		case cCompletedThreadStatue_WaitQueue:
			free(pContext->pWaitCommandItemInstance);
		
		case cCompletedThreadStatue_WaitCommandItemList:
			free(pContext);

		case cCompletedThreadStatue_Instance:
			break;
		default:
			ASSERT(0);
			break;
	}


}



static void CompletedThread_Exit(CompletedThreadContext_t *pContext)
{
	DebugPrint("start");
	pContext->exit = true;
	// abort all the request

}

static void *CompletedThread_Processor(void *param)
{

	CompletedThreadContext_t *pContext = (CompletedThreadContext_t *)param;
	int count = 0;
	DebugPrint("start");
	pContext->ready = true;

	while (true)
	{
		clock_t start = clock();
		WaitCommandItem_t *pItem = NULL;
		while ((count != 0) || ((count = CommandIdFifo_Count(&pContext->waitQueue)) != 0))
		{
			// DebugPrint("wait queue count: %d", count);
			ASSERT_DEBUG(count != 0);
			// alloc the wait item
			uint_t allocCount = SimpleList_PopFromHead(&pContext->waitCommandItemFreeList, 1, (void **)&pItem);
			ASSERT_DEBUG(allocCount == 1);
			// pop the command id
			bool ok = CommandIdFifo_Pop(&pContext->waitQueue, &pItem->commandId);
			ASSERT_DEBUG(ok);

			DebugPrint("to wait: %d", pItem->commandId);

			pContext->waitCount ++;

			// push to wait item list
			SimpleList_PushToTail(&pContext->waitCommandItemList, pItem, pItem, 1);
			DebugPrint("wait list: %d/%d %d", ((WaitCommandItem_t *)(pContext->waitCommandItemList.pHead))->commandId, ((WaitCommandItem_t *)(pContext->waitCommandItemList.pTail))->commandId, SimpleList_Count(&pContext->waitCommandItemList));
			count --;
		}

		// count = SimpleList_Count(&pContext->waitCommandItemList);
		WaitCommandItem_t *pNextItem = pContext->waitCommandItemList.pHead;
		if (pNextItem != (void *)(cSimpleListInvalid))
		{
			DebugPrint("wait head: %d", pNextItem->commandId);
		}

		while (pNextItem != (void *)(cSimpleListInvalid))
		{
			// get the wait item
			pItem = pNextItem;
			pNextItem = pItem->pNext;

			// get the command
			CommonCommand_t *pCommand = CommonCommandPool_GetCommand(pItem->commandId);
			DebugPrint("P %d/%ld", pItem->commandId, pNextItem != (void *)(cSimpleListInvalid) ? pNextItem->commandId : cSimpleListInvalid);
			
			bool isCompleted = false;
			unsigned int passMs;
			// check the command statue
			switch (pCommand->config.status)
			{
			case cCommandStatus_Abort:
			case cCommandStatus_Failed:
			case cCommandStatus_Success:
				isCompleted = true;
				break;
			case cCommandStatus_Invalid:
				ASSERT(0);
				break;
			
			default:
				unsigned int passMs = (start - pCommand->time.submitTime) / (CLOCKS_PER_SEC / 1000);
				if (passMs > pCommand->time.timeoutMs)
				{
					isCompleted = true;
					// timeout
					// do timeout
					// move to free list
					pCommand->config.status = cCommandStatus_Timeout;
				}
				break;
			}
			

			if (isCompleted)
			{
				DebugPrint("Completed: %d %d", pItem->commandId, pCommand->config.status);
				pContext->completedCount ++;
				// move to completed queue
				while (CommandIdFifo_Push(&pContext->completedQueue, pItem->commandId) == false)
				{
					// wait for completed queue free
					usleep(10);
				}
				// isCompleted =  CommandIdFifo_Push(&pContext->completedQueue, pItem->commandId);
				// ASSERT_DEBUG(isCompleted);

				SimpleList_Dell(&pContext->waitCommandItemList, pItem);
				SimpleList_PushToTail(&pContext->waitCommandItemFreeList, pItem, pItem, 1);

			}
		}

		if (pContext->exit && CommandIdFifo_IsEmpty(&pContext->waitQueue) && SimpleList_Count(&pContext->waitCommandItemList) == 0)
		{
			break;
		}
	}
	DebugPrint("exit");
	pthread_exit(NULL);
	return NULL;
}

static void *WaitCommandItemGetNext(const WaitCommandItem_t *pItem)
{
	ASSERT_DEBUG(pItem != NULL && (void *)pItem != (void *)(cSimpleListInvalid));
	return pItem->pNext;
}


static void WaitCommandItemSetNext(WaitCommandItem_t *pItem, void *pNext)
{
	// DebugPrint("set next %p/%p", pItem, pNext);
	ASSERT_DEBUG(pItem != NULL && (void *)pItem != (void *)(cSimpleListInvalid));
	pItem->pNext = pNext;
}


static bool PushToWaitQueue(CompletedThreadContext_t *pContext, CommandId_t commandId)
{
	return CommandIdFifo_Push(&pContext->waitQueue, commandId);
}




//-----------------------------------------------------------------------------
//  Public functions
//-----------------------------------------------------------------------------

IoEngine_t *IoEngine_Create(uint_t ioQueueDepth)
{
	IoEngine_t *pIoEngine = malloc(sizeof(IoEngine_t));
	if (pIoEngine == NULL)
	{
		DebugPrint("IoEngine malloc error");
		return NULL;
	}

	pIoEngine->requestThreadContext = RequestThreadContext_Creat(ioQueueDepth, pIoEngine);
	if (pIoEngine->requestThreadContext == NULL)
	{
		DebugPrint("RequestThreadContext  error");
		goto Failed;
	}


	pIoEngine->completedThreadContext = CompletedThreadContext_Creat(ioQueueDepth, pIoEngine);
	if (pIoEngine->completedThreadContext == NULL)
	{
		RequestThread_Destroy(pIoEngine->requestThreadContext, cRequestThreadStatue_Completed);
		DebugPrint("CompletedThreadContext  error");
		goto Failed;
	}

	return pIoEngine;

Failed:
	return NULL;
}


void IoEngine_Run(IoEngine_t *pIoEngine)
{
	if (pIoEngine == NULL)
	{
		return;
	}
	RequestThreadContext_t *pContext = pIoEngine->requestThreadContext;
	CompletedThreadContext_t *pCompletedContext = pIoEngine->completedThreadContext;

	while ((pContext->ready == false) || (pCompletedContext->ready == false))
	{
		sleep(1);
	}
	DebugPrint("Ready");
}

void IoEngine_Destroy(IoEngine_t *pIoEngine)
{
	if (pIoEngine == NULL)
	{
		return;
	}

	RequestThreadContext_t *pContext = pIoEngine->requestThreadContext;
	RequestThread_Destroy(pContext, cRequestThreadStatue_Completed);

	CompletedThreadContext_t *pCompletedContext = pIoEngine->completedThreadContext;
	CompletedThreadContext_Destroy(pCompletedContext, cCompletedThreadStatue_Completed);

	free(pIoEngine);
}


bool IoEngine_Submit(IoEngine_t *pIoEngine, CommandId_t commandId)
{
	// DebugPrint("start");
	if (pIoEngine == NULL)
	{
		return false;
	}

	RequestThreadContext_t *pContext = pIoEngine->requestThreadContext;
	// abort always return false
	if (pContext->abort || pContext->exit)
	{
		return false;
	}

	// DebugPrint("push");
	bool notify = CommandIdFifo_IsEmpty(&pContext->requestQueue);
	bool ret = CommandIdFifo_Push(&pContext->requestQueue, commandId);

	// level trigger, don't not edge trigger
	// if (notify)
	{
		// DebugPrint("notify");
		RequestThread_Notify(pContext);
	}
	return ret;
}

uint_t IoEngine_RequestQueueFreeCount(const IoEngine_t *pIoEngine)
{
	if (pIoEngine == NULL)
	{
		return 0;
	}

	RequestThreadContext_t *pContext = pIoEngine->requestThreadContext;
	return pContext->abort ? 0 : CommandIdFifo_FreeCount(&pContext->requestQueue);
}


void IoEngine_AbortRequestQueue(IoEngine_t *pIoEngine)
{
	if (pIoEngine == NULL)
	{
		return;
	}

	RequestThreadContext_t *pContext = pIoEngine->requestThreadContext;
	RequestThread_Abort(pContext);

}

bool IoEngine_RequestQueueIsAbort(IoEngine_t *pIoEngine)
{
	if (pIoEngine == NULL)
	{
		return false;
	}

	RequestThreadContext_t *pContext = pIoEngine->requestThreadContext;
	return pContext->abort;

}

void IoEngine_ResetRequestQueue(IoEngine_t *pIoEngine)
{
	if (pIoEngine == NULL)
	{
		return;
	}

	RequestThreadContext_t *pContext = pIoEngine->requestThreadContext;
	RequestThread_Abort(pContext);

	while (CommandIdFifo_IsEmpty(&pContext->requestQueue) == false)
	{
		// wait for abort
		sleep(1);
	}

	// reset the abort flag, can't process the request again
	pContext->abort = false;
}



uint_t IoEngine_CompletedQueueCount(const IoEngine_t *pIoEngine)
{
	if (pIoEngine == NULL)
	{
		return 0;
	}

	CompletedThreadContext_t *pContext = pIoEngine->completedThreadContext;
	return CommandIdFifo_Count(&pContext->completedQueue);
}

bool IoEngine_CompletedQueuePop(IoEngine_t *pIoEngine, CommandId_t *pCommandId)
{
	if (pIoEngine == NULL)
	{
		return false;
	}

	CompletedThreadContext_t *pContext = pIoEngine->completedThreadContext;
	return CommandIdFifo_Pop(&pContext->completedQueue, pCommandId);
}
