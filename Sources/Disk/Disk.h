#pragma once


#ifdef __cplusplus
extern "C" {
#endif
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <limits.h>

typedef struct Disk
{
	char filePath[PATH_MAX];
	char verifyFilePath[PATH_MAX];
	void *pVerifyDataBaseAddr;
	int verifyFileSize;

	int diskFileHandler;
	int verifyFileHandler;
	uint64_t maxSectorCount;
} Disk_t;


typedef enum LbaStatue
{
	cLbaStatue_NoInit = 0,		// mean's this lba not be initialized
	cLbaStatue_Invalid,			// mean's this lba be trim or formate
	cLbaStatue_Valid,			// mean's this lba be this io engine write, the data pattern has certain pattern
} LbaStatue_t;

typedef union LbaData
{
	uint32_t all;
	struct
	{
		bool isStatistic		: 1;		// mean's lba modify command send but not completed	
		LbaStatue_t statue		: 2;		// lba state
		uint32_t writeCount		: 29;		// this lba write count, using lba write count as verify data, when write the new lba buffer will be fill with lba + writeCount + 1, when write completed, it will be set as writeCount + 1. when read completed writeCount used to verify data.
	};
} LbaData_t;



Disk_t *Disk_Create(const char *filePath, const char *verifyFilePath, uint64_t maxSectorCount);
void Disk_Destroy(Disk_t *pDisk);


#ifdef __cplusplus
}
#endif