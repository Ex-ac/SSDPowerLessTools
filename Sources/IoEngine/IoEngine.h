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

typedef struct IoEngine
{
	void *requestThreadContext;
	void *completedThreadContext;
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

IoEngine_t *IoEngine_Create(uint_t ioQueueDepth);
void IoEngine_Run(IoEngine_t *pIoEngine);
void IoEngine_Destroy(IoEngine_t *pIoEngine);

bool IoEngine_Submit(IoEngine_t *pIoEngine, const CommonCommand_t *pCommand);
uint_t IoEngine_RequestQueueCount(IoEngine_t *pIoEngine);
void IoEngine_AbortRequestQueue(IoEngine_t *pIoEngine);
bool IoEngine_RequestQueueIsAbort(IoEngine_t *pIoEngine);
void IoEngine_ResetRequestQueue(IoEngine_t *pIoEngine);

//-----------------------------------------------------------------------------
//  Inline functions
//-----------------------------------------------------------------------------


#ifdef __cplusplus
}
#endif
