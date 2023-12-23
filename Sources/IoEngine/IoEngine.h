#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "SimpleFifo.h"
#include <pthread.h>



typedef enum IoEngineType
{
	cIoEngineType_Default = 0x00,
	cIoEngineType_Aio,
} IoEngineType_t;


typedef struct IoEngine
{
	bool exit;
	void *requestQueue;
	void *completedQueue;
	uint_t ioQueueDepth;
	IoEngineType_t ioQueueDepth;
	pthread_t submitThread;
	pthread_t completionThread;
	
} IoEngine_t;


IoEngine_t *IoEngine_Create(uint_t ioQueueDepth);
void IoEngine_Run(IoEngine_t *pIoEngine);
void IoEngine_Destroy(IoEngine_t *pIoEngine);



#ifdef __cplusplus
}
#endif