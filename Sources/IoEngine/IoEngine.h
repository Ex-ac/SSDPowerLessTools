#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "SimpleFifo.h"
#include <pthread.h>
#include <semaphore.h>



typedef enum IoEngineType
{
	cIoEngineType_Default = 0x00,
	cIoEngineType_Aio,
} IoEngineType_t;


typedef struct RequestThreadContext
{
	sem_t semaphore;		// when queue empty to not empty will post
	void *requestQueue;		// to save the usr request
	pthread_t thread;
	bool abort				: 1;	// return all the requests to completed queue
	bool exit				: 1;	// do abort and exit the thread
} RequestThreadContext_t;

typedef struct IoEngine
{
	bool exit;
	sem_t requestQueueSemaphore,
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