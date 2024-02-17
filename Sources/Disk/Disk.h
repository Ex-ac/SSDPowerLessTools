
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
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <limits.h>
#include <pthread.h>
#include "Type.h"


//-----------------------------------------------------------------------------
//  Constant definitions:
//-----------------------------------------------------------------------------




//-----------------------------------------------------------------------------
//  Macros definitions:
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
//  Data type definitions: typedef, struct or class
//-----------------------------------------------------------------------------

typedef struct Disk
{
	char filePath[PATH_MAX];
	char verifyFilePath[PATH_MAX];
	void *pVerifyDataBaseAddr;
	int verifyFileSize;

	int diskFileHandler;
	int verifyFileHandler;
	uint64_t maxSectorCount;
	pthread_mutex_t mutex;
} Disk_t;



//-----------------------------------------------------------------------------
//  Public interface functions:
//-----------------------------------------------------------------------------


Disk_t *Disk_Create(const char *filePath, const char *verifyFilePath, uint64_t maxSectorCount);
void Disk_Destroy(Disk_t *pDisk);
void Disk_VerifyFileDeInit(Disk_t *pDisk);
void *Disk_GetLbaVerifyDataAddr(const Disk_t *pDisk, uint64_t lba);



//-----------------------------------------------------------------------------
//  Inline functions
//-----------------------------------------------------------------------------


static inline uint64_t Disk_GetMaxSectorCount(const Disk_t *pDisk)
{
	return pDisk->maxSectorCount;
}


static inline bool Disk_TryLock(Disk_t *pDisk)
{
	return pthread_mutex_trylock(&pDisk->mutex) == 0;
}

bool Disk_Lock(Disk_t *pDisk)
{
	return pthread_mutex_lock(&pDisk->mutex) == 0;
}

bool Disk_Unlock(Disk_t *pDisk)
{
	return pthread_mutex_unlock(&pDisk->mutex) == 0;
}

#ifdef __cplusplus
}
#endif

