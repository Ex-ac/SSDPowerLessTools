#define _GNU_SOURCE

#include "Disk.h"
#include <string.h>
#include <assert.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/mman.h>

#define PAGE_ALIGN(x)    (((x + getpagesize() - 1) / getpagesize()) * getpagesize())


const char cVerifyExpendName[] = ".VD";
const LbaData_t cNoInitLbaData = {
	.isStatistic = true,
	.statue = cLbaStatue_NoInit,
	.writeCount = 0x1FFFFFFF,
};

Disk_t *Disk_Create(const char *filePath, const char *verifyFilePath, uint64_t maxSectorCount)
{
	bool indicateVerifyPath = verifyFilePath != NULL;
	
	assert(maxSectorCount > 0);
	assert(filePath != NULL);
	int filePathSize = strlen(filePath);
	assert(filePathSize < PATH_MAX);

	if (indicateVerifyPath)
	{
		assert(strlen(verifyFilePath) < PATH_MAX);
	}


	assert(verifyFilePath == NULL || strlen(verifyFilePath) < PATH_MAX);

	Disk_t *pDisk = malloc(sizeof(Disk_t));

	if (pDisk == NULL)
	{
		return NULL;
	}

	memset(pDisk, 0, sizeof(Disk_t));

	pDisk->pVerifyDataBaseAddr = MAP_FAILED;

	// save test dike file name
	strcpy(pDisk->filePath, filePath);

	// save verify data file name
	// if indicate verify file path, use it, else use 
	if (indicateVerifyPath)
	{
		strcpy(pDisk->verifyFilePath, verifyFilePath);
	}
	else
	{
		char *pStart = strrchr(filePath, '/');
		pStart += 1;
		strcpy(pDisk->verifyFilePath, pStart);
		strcpy(pDisk->verifyFilePath + strlen(pStart), cVerifyExpendName);
	}

	fprintf(stdout, "Disk File Name: %s, Verify File Name: %s\n", pDisk->filePath, pDisk->verifyFilePath);
	

	pDisk->diskFileHandler = open(pDisk->filePath, O_RDWR | O_DIRECT, S_IRWXU);

	if (pDisk->diskFileHandler < 0)
	{
		fprintf(stderr, "Open Disk file failed\n");
		goto ErrorOut;
	}
	fprintf(stdout, "Open Disk file success\n");

	if (indicateVerifyPath)
	{
		pDisk->verifyFileHandler = open(pDisk->verifyFilePath, O_RDWR | O_DIRECT, S_IRWXU);
	}
	else
	{
		pDisk->verifyFileHandler = open(pDisk->verifyFilePath, O_RDWR | O_CREAT, S_IRWXU);
	}

	if (pDisk->verifyFileHandler < 0)
	{
		fprintf(stderr, "Open Verify file failed\n");
		goto ErrorOut;
	}

	fprintf(stdout, "Open Verify file success\n");

	pDisk->maxSectorCount = maxSectorCount;
	pDisk->verifyFileSize = PAGE_ALIGN(maxSectorCount * sizeof(LbaData_t));
	ftruncate(pDisk->verifyFileHandler, pDisk->verifyFileSize);

	pDisk->pVerifyDataBaseAddr = mmap(NULL, pDisk->verifyFileSize, PROT_READ | PROT_WRITE, MAP_SHARED, pDisk->verifyFileHandler, 0);
	if (pDisk->pVerifyDataBaseAddr == MAP_FAILED)
	{
		fprintf(stderr, "map Verify file failed\n");
		goto ErrorOut;
	}

	fprintf(stdout, "map verify file success\n");

	if (!indicateVerifyPath)
	{
		int pageNum = pDisk->verifyFileSize / getpagesize();
	

		LbaData_t *pLbaData = (LbaData_t*)(pDisk->pVerifyDataBaseAddr);

		for (int i = 0; i < getpagesize() / sizeof(LbaData_t); ++i)
		{
			pLbaData->all = cNoInitLbaData.all;
			pLbaData ++;
		}

		void *pDest;
		for (int i = 1; i < pageNum; ++i)
		{
			pDest = pDisk->pVerifyDataBaseAddr + i * getpagesize();
			memcpy(pDest, (void *)(pDisk->pVerifyDataBaseAddr), getpagesize());
		}
	}
	

	fprintf(stdout, "Disk create success\n");


	return pDisk;
ErrorOut:
	Disk_Destroy(pDisk);
	return NULL;
}


void Disk_Destroy(Disk_t *pDisk)
{
	if (pDisk == NULL)
	{
		return;
	}

	if (pDisk->diskFileHandler > 0)
	{
		close(pDisk->diskFileHandler);
	}

	if (pDisk->verifyFileHandler > 0)
	{
		if (pDisk->pVerifyDataBaseAddr != MAP_FAILED)
		{
			munmap(pDisk->pVerifyDataBaseAddr, pDisk->verifyFileSize);
		}
		close(pDisk->verifyFileHandler);
	}

	free(pDisk);
}

