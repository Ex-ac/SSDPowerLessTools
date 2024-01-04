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

#include "debug.h"
#include "SimpleFifo.h"

//-----------------------------------------------------------------------------
//  Constants definitions:
//-----------------------------------------------------------------------------

typedef enum RequestThreadStatue
{
	cRequestThreadStatue_Instance = 0x00,
	cRequestThreadStatue_Semaphore,
	cRequestThreadStatue_Queue,
	cRequestThreadStatue_Thread,
	cRequestThreadStatue_Completed,
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
	void *requestQueue;		// to save the usr request
	pthread_t thread;
	bool abort				: 1;	// return all the requests to completed queue
	bool exit				: 1;	// do abort and exit the thread
} RequestThreadContext_t;

//-----------------------------------------------------------------------------
//  Private function proto-type definitions:
//-----------------------------------------------------------------------------
static RequestThreadContext_t *RequestThreadContext_Creat(uint_t queueDepth);
static void RequestThread_Destroy(RequestThreadContext_t *pContext, int statue);
static void *RequestThread_Processor(void *param);
static void RequestThread_Exit(RequestThreadContext_t *pContext);

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


//-----------------------------------------------------------------------------
//  Public functions
//-----------------------------------------------------------------------------















static void *RequestThread_Processor(void *param)
{
	RequestThreadContext_t *pContext = (RequestThreadContext_t *)param;
	DebugPrint("start");
	while (true)
	{
		DebugPrint("wait semaphore");
		sem_wait(&pContext->semaphore);
		DebugPrint("get semaphore");

		if (pContext->exit)
		{
			break;
		}
		else if (pContext->abort)
		{
			DebugPrint("abort");
		}
		else
		{
			// processor queue
		}
	}
	fprint(stdout, "exit\n");
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

	status ++;

	memset(pContext, 0, sizeof(RequestThreadContext_t));
	sem_init(&pContext->semaphore, 0, 0);
	pContext->thread = malloc(sizeof(pthread_t));
	if (pContext->thread == NULL)
	{
		goto Failed;
	}
	status ++;

	// create queue
	status ++;

	pthread_create(pContext->thread, NULL, RequestThread_Processor, pContext);
	status ++;

	return pContext;

Failed:
	
	return NULL;
}

static void RequestThread_Exit(RequestThreadContext_t *pContext);

static void RequestThread_Destroy(RequestThreadContext_t *pContext, int statue)
{
	if (pContext == NULL)
	{
		return;
	}

	switch (statue)
	{
		case cRequestThreadStatue_Completed:
			RequestThread_Exit(pContext);
			pthread_join(pContext->thread, NULL);
			free(pContext->thread);
			pContext->thread = NULL;
		
		case cRequestThreadStatue_Thread:
			// free(pContext->requestQueue);
			pContext->requestQueue = NULL;

		case cRequestThreadStatue_Queue:
			//sem_destroy(&pContext->semaphore);
		
		case cRequestThreadStatue_Semaphore:
			free(pContext);

		case cRequestThreadStatue_Instance:
			break;
		default:
			assert(0);
			break;
	}

}



IoEngine_t *IoEngine_Create(uint_t ioQueueDepth)
{
	IoEngine_t *pIoEngine = malloc(sizeof(IoEngine_t));
	if (pIoEngine == NULL)
	{
		DebugPrint("IoEngine malloc error");
		return NULL;
	}
}


