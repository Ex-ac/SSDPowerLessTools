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
	cCompletedThreadStatue_Instance = 0x00,	// alloc the instance
	cCompletedThreadStatue_WaitCommandItemList,		// init the wait item list
	cCompletedThreadStatue_Queue,				// init the queue
	cCompletedThreadStatue_Epoll,				// init the epoll
	cCompletedThreadStatue_Thread,			// create the thread
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
	sem_t semaphore;		// when queue empty to not empty will post
	CommandIdFifo_t requestQueue;	// to save the usr request
	pthread_t thread;
	bool abort				: 1;	// return all the requests to completed queue
	bool exit				: 1;	// do abort and exit the thread
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
	SimpleList_t waitCommandItemList;
	WaitCommandItem_t *pWaitCommandItemList;
	CommandIdFifo_t completedQueue;		// to save the complete from driver
	CommandIdFifo_t waitQueue;			// wait for complete
	pthread_t thread;
	struct epoll_event epollEvent;
	int epollFileHandler;
	bool exit				: 1;	// when completed queue empty, exit the thread
} CompletedThreadContext_t;







//-----------------------------------------------------------------------------
//  Private function proto-type definitions:
//-----------------------------------------------------------------------------
/**
 * @brief create request thread context
 * 
 * @param queueDepth 
 * @return when create success, return the context pointer, else return NULL
 */
static RequestThreadContext_t *RequestThreadContext_Creat(uint_t queueDepth);

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



static CompletedThreadContext_t *CompletedThreadContext_Creat(uint_t queueDepth);

static void CompletedThreadContext_Destroy(CompletedThreadContext_t *pContext, int statue);

static void CompletedThread_Exit(CompletedThreadContext_t *pContext);

static void *CompletedThread_Processor(void *param);

static void *WaitCommandItemGetNext(const WaitCommandItem_t *pItem);
static void WaitCommandItemSetNext(WaitCommandItem_t *pItem, void *pNext);


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
	CommandId_t commandId;
	while (true)
	{
		DebugPrint("wait semaphore");
		sem_wait(&pContext->semaphore);
		DebugPrint("get semaphore");
		
		
		while ((count != 0) || (count = CommandIdFifo_Count(&pContext->requestQueue)) != 0)
		{
			bool ok = CommandIdFifo_Pop(&pContext->requestQueue, &commandId);
			ASSERT_DEBUG(ok);

			if (pContext->exit || pContext->abort)
			{
				// move to completed queue

			}
			else
			{
				// do the request
			}
			DebugPrint("process request. command id: %d", commandId);

			usleep(10000);
			count --;
		}
		DebugPrint("queue empty");

		if (pContext->exit)
		{
			break;
		}
	}
	DebugPrint("exit");
	pthread_exit(NULL);
	return NULL;
}

static RequestThreadContext_t *RequestThreadContext_Creat(uint_t queueDepth)
{
	RequestThreadStatue_t status = cRequestThreadStatue_Instance;
	RequestThreadContext_t *pContext = malloc(sizeof(RequestThreadContext_t));
	if (pContext == NULL)
	{
		goto Failed;
	}
	DebugPrint("Context malloc %p/%p", pContext, &pContext->requestQueue);
	status ++;
	pContext->abort = true;

	memset(pContext, 0, sizeof(RequestThreadContext_t));
	sem_init(&pContext->semaphore, 0, 0);

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

	return pContext;

Failed:
	RequestThread_Destroy(pContext, status);
	return NULL;
}


static void RequestThread_Destroy(RequestThreadContext_t *pContext, int statue)
{
	DebugPrint("start");
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
	sem_post(&pContext->semaphore);
}




static CompletedThreadContext_t *CompletedThreadContext_Creat(uint_t queueDepth)
{

}

static void CompletedThreadContext_Destroy(CompletedThreadContext_t *pContext, int statue);

static void CompletedThread_Exit(CompletedThreadContext_t *pContext);

static void *COmpletedThread_Processor(void *param);


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

	pIoEngine->requestThreadContext = RequestThreadContext_Creat(ioQueueDepth);
	if (pIoEngine->requestThreadContext == NULL)
	{
		DebugPrint("RequestThreadContext  error");
		goto Failed;
	}

	return pIoEngine;

Failed:
	return NULL;
}

void IoEngine_Destroy(IoEngine_t *pIoEngine)
{
	if (pIoEngine == NULL)
	{
		return;
	}

	RequestThreadContext_t *pContext = pIoEngine->requestThreadContext;
	RequestThread_Destroy(pContext, cRequestThreadStatue_Completed);
	free(pIoEngine);
}


bool IoEngine_Submit(IoEngine_t *pIoEngine, CommandId_t commandId)
{
	DebugPrint("start");
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

	DebugPrint("push");
	bool notify = CommandIdFifo_IsEmpty(&pContext->requestQueue);
	bool ret = CommandIdFifo_Push(&pContext->requestQueue, commandId);
	if (notify)
	{
		DebugPrint("notify");
		RequestThread_Notify(pContext);
	}
	return ret;
}

uint_t IoEngine_RequestQueueFreeCount(IoEngine_t *pIoEngine)
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



static void *WaitCommandItemGetNext(const WaitCommandItem_t *pItem)
{
	ASSERT_DEBUG(pItem != NULL && (void *)pItem != (void *)(cSimpleListInvalid));
	return pItem->pNext;
}


static void WaitCommandItemSetNext(WaitCommandItem_t *pItem, void *pNext)
{
	ASSERT_DEBUG(pItem != NULL && (void *)pItem != (void *)(cSimpleListInvalid));
	pItem->pNext = pNext;
}