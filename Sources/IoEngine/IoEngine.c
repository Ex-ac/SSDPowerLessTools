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


#include "IoEngine.h"
#include <pthread.h>
#include <semaphore.h>
#include <memory.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>


#include "SimpleFifo.h"
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

//-----------------------------------------------------------------------------
//  Macros definitions:
//-----------------------------------------------------------------------------

#ifdef Uint
#undef Uint
#endif
// using the unsigned int to save addr
#define Uint			uint_t

SIMPLE_FIFO_TYPE_DEFINE_GEN(Uint);
SIMPLE_FIFO_TYPE_INIT_GEN(Uint);
SIMPLE_FIFO_TYPE_COUNT_GEN(Uint);
SIMPLE_FIFO_TYPE_IS_FULL_GEN(Uint);
SIMPLE_FIFO_TYPE_IS_EMPTY_GEN(Uint);
SIMPLE_FIFO_TYPE_POP_GEN(Uint);
SIMPLE_FIFO_TYPE_PUSH_GEN(Uint);

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
	Queue_t requestQueue;		// to save the usr request
	pthread_t thread;
	bool abort				: 1;	// return all the requests to completed queue
	bool exit				: 1;	// do abort and exit the thread
} RequestThreadContext_t;


typedef struct CompletedThreadContext
{

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
 * @brief when queue not empty, notify the request thread wake up
 * 
 * @param pQueue 
 */
static void RequestThread_Notify(Queue_t *pQueue);
//-----------------------------------------------------------------------------
//  Data declaration: Private or Public
//-----------------------------------------------------------------------------


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
	while (true)
	{
		DebugPrint("wait semaphore");
		sem_wait(&pContext->semaphore);
		DebugPrint("get semaphore");
		
		
		while ((count != 0) || (count = Queue_Count(&pContext->requestQueue)) != 0)
		{
			CommonCommand_t *pCommand = Queue_Peek(&pContext->requestQueue);
			if (pContext->exit || pContext->abort)
			{
				// move to completed queue

			}
			else
			{
				// do the request
			}
			Queue_Remove(&pContext->requestQueue);
			DebugPrint("process request %p command id: %d", pCommand, pCommand->config.commandId);

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
	SimpleFifo_t *pSimpleFifo = SimpleFifo_Create(queueDepth, sizeof(CommonCommand_t));
	if (pSimpleFifo == NULL)
	{
		goto Failed;
	}
	Queue_Init(&pContext->requestQueue, pSimpleFifo, &cSimpleFifoOperator);
	status ++;
	
	Queue_RegisterEventHandler(&pContext->requestQueue, cQueueEventType_ChangeToNotEmpty, RequestThread_Notify);

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
			SimpleFifo_Destroy(pContext->requestQueue.pInstance);

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
	RequestThread_Notify(&pContext->requestQueue);
	// abort all the request
}

static void RequestThread_Abort(RequestThreadContext_t *pContext)
{
	DebugPrint("start");
	pContext->abort = true;
	
	// abort all the request
}

static void RequestThread_Notify(Queue_t *pQueue)
{
	DebugPrint("start");
	RequestThreadContext_t *pContext = CONTAINER_OF(pQueue, RequestThreadContext_t, requestQueue);
	// DebugPrint("Context %p/%p/%p", pContext, &pContext->requestQueue, pQueue);
	sem_post(&pContext->semaphore);
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


bool IoEngine_Submit(IoEngine_t *pIoEngine, const CommonCommand_t *pCommand)
{
	DebugPrint("start");
	if (pIoEngine == NULL || pCommand == NULL)
	{
		return false;
	}

	RequestThreadContext_t *pContext = pIoEngine->requestThreadContext;
	Queue_t *pQueue = &pContext->requestQueue;
	// abort always return false
	if (pContext->abort || pContext->exit)
	{
		return false;
	}

	DebugPrint("push");
	return Queue_Push(pQueue, (void *)pCommand);
}

uint_t IoEngine_RequestQueueFreeCount(IoEngine_t *pIoEngine)
{
	if (pIoEngine == NULL)
	{
		return 0;
	}

	RequestThreadContext_t *pContext = pIoEngine->requestThreadContext;
	Queue_t *pQueue = &pContext->requestQueue;
	DebugPrint("%d", Queue_FreeCount(pQueue));
	return pContext->abort ? 0 : Queue_FreeCount(pQueue);
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

	while (Queue_Count(&pContext->requestQueue) != 0)
	{
		// wait for abort
		sleep(1);
	}

	// reset the abort flag, can't process the request again
	pContext->abort = false;
}