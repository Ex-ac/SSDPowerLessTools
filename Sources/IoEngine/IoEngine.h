/**
 * @file IoEngine.h
 * @author Exac (ex-ac@outlook.com)
 * @brief 
 * @version 0.1
 * @date 2024-01-03
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#pragma once


#ifdef __cplusplus
extern "C" {
#endif


//-----------------------------------------------------------------------------
//  Include files:
//-----------------------------------------------------------------------------
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "Type.h"

//-----------------------------------------------------------------------------
//  Constant definitions:
//-----------------------------------------------------------------------------
#define IO_ENGINE_DEBUG				true

typedef enum IoEngineType
{
	cIoEngineType_Default = 0x00,
	cIoEngineType_Aio,
} IoEngineType_t;

//-----------------------------------------------------------------------------
//  Macros definitions:
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
//  Data type definitions: typedef, struct or class
//-----------------------------------------------------------------------------
struct RequestThreadContext;
struct CompletedThreadContext;
struct EngineOperator;
typedef struct IoEngine
{
	struct RequestThreadContext *requestThreadContext;
	struct CompletedThreadContext *completedThreadContext;
	const struct EngineOperator *engineOperator;
	// bool exit;
	// sem_t requestQueueSemaphore,
	// void *requestQueue;
	// void *completedQueue;
	// uint_t ioQueueDepth;
	// IoEngineType_t ioQueueDepth;
	// pthread_t submitThread;
	// pthread_t completionThread;
	
} IoEngine_t;

//-----------------------------------------------------------------------------
//  Public interface functions:
//-----------------------------------------------------------------------------

/**
 * @brief create io engine
 * 
 * @param ioQueueDepth request queue depth
 * @return when create success, return the io engine pointer, else return NULL
 */
IoEngine_t *IoEngine_Create(uint_t ioQueueDepth);


void IoEngine_Run(IoEngine_t *pIoEngine);

/**
 * @brief destroy io engine, when destroy, will abort the request queue
 * 
 * @param pIoEngine 
 */
void IoEngine_Destroy(IoEngine_t *pIoEngine);

/**
 * @brief submit io command to io engine
 * 
 * @param pIoEngine 
 * @param pCommand 
 * @return true io command submit success
 * @return false io command submit fail, queue is full or abort
 */
bool IoEngine_Submit(IoEngine_t *pIoEngine, CommandId_t commandId);

/**
 * @brief get the request queue free count
 * 
 * @param pIoEngine 
 * @return free count
 */
uint_t IoEngine_RequestQueueFreeCount(const IoEngine_t *pIoEngine);

/**
 * @brief abort the request queue, incomplete io command will return to completed queue
 * 
 * @param pIoEngine 
 */
void IoEngine_AbortRequestQueue(IoEngine_t *pIoEngine);

/**
 * @brief check the request queue is abort
 * 
 * @param pIoEngine 
 * @return true request queue is abort
 * @return false request queue is not abort
 */
bool IoEngine_RequestQueueIsAbort(IoEngine_t *pIoEngine);

/**
 * @brief abort all the request queue, incomplete io command will return to completed queue, when request is empty, can send new request again
 * 
 * @param pIoEngine 
 */
void IoEngine_ResetRequestQueue(IoEngine_t *pIoEngine);


uint_t IoEngine_CompletedQueueCount(const IoEngine_t *pIoEngine);
bool IoEngine_CompletedQueuePop(IoEngine_t *pIoEngine, CommandId_t *pCommandId);
//-----------------------------------------------------------------------------
//  Inline functions
//-----------------------------------------------------------------------------


#ifdef __cplusplus
}
#endif
