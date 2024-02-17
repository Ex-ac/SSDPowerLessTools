
#define _GNU_SOURCE

//-----------------------------------------------------------------------------
//  Include files:
//-----------------------------------------------------------------------------
#include "Disk.h"
#include "Debug.h"
#include <string.h>
#include <assert.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/mman.h>

//-----------------------------------------------------------------------------
//  Constants definitions:
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
//  Macros definitions:
//-----------------------------------------------------------------------------
#define PAGE_ALIGN(x)    (((x + getpagesize() - 1) / getpagesize()) * getpagesize())

//-----------------------------------------------------------------------------
//  Data type definitions: typedef, struct or class
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
//  Private function proto-type definitions:
//-----------------------------------------------------------------------------
static void IoCommandCheck(const CommonCommand_t *pCommand);

static void VerifyWritePrepareFunc(CommonCommand_t *pCommand);
static void VerifyWriteCompletedFunc(CommonCommand_t *pCommand);


static void VerifyReadPrepareFunc(CommonCommand_t *pCommand);
static void VerifyReadCompletedFunc(CommonCommand_t *pCommand);

//-----------------------------------------------------------------------------
//  Data declaration: Private or Public
//-----------------------------------------------------------------------------

const char cVerifyExpendName[] = ".VD";
const LbaData_t cNoInitLbaData = {
	.isStatistic = true,
	.statue = cLbaStatue_NoInit,
	.writeCount = 0x1FFFFFFF,
};

const CommonCommandPrepareAndCompletedFunc_t cVerifyWritePrepareAndCompletedFunc = {
	.prepareFunc = VerifyWritePrepareFunc,
	.completedFunc = VerifyWriteCompletedFunc,
};

const CommonCommandPrepareAndCompletedFunc_t cVerifyReadPrepareAndCompletedFunc = {
	.prepareFunc = VerifyReadPrepareFunc,
	.completedFunc = VerifyReadCompletedFunc,
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
static void IoCommandCheck(const CommonCommand_t *pCommand)
{
	ASSERT_DEBUG(pCommand->ioRequest.pDisk != NULL);
	ASSERT_DEBUG(pCommand->ioRequest.buffer != NULL);
	ASSERT_DEBUG(pCommand->ioRequest.lbaRange.sectorCount != 0);
	ASSERT_DEBUG(pCommand->ioRequest.lbaRange.sectorCount < cMaxSectorInCommand);
	ASSERT_DEBUG(pCommand->ioRequest.lbaRange.startLba + pCommand->ioRequest.lbaRange.sectorCount <= pCommand->ioRequest.pDisk->maxSectorCount);
}


static void VerifyWritePrepareFunc(CommonCommand_t *pCommand)
{
	ASSERT_DEBUG(pCommand != NULL);
	ASSERT_DEBUG(pCommand->config.type == cCommandType_Io);
	ASSERT_DEBUG(pCommand->ioRequest.ioType == cIoType_Write);

	
	Disk_t *pDisk = pCommand->ioRequest.pDisk;
	uint64_t startLba = pCommand->ioRequest.lbaRange.startLba;
	int sectorCount = pCommand->ioRequest.lbaRange.sectorCount;

	LbaVerifyHeader_t verifyHeader;

	LbaData_t *pLbaData = Disk_GetLbaVerifyDataAddr(pDisk, startLba);
	memcpy((void *)(pCommand->ioRequest.verifyLbaData), (void *)(pLbaData), pCommand->ioRequest.lbaRange.sectorCount * sizeof(LbaData_t));

	for (int sectorIndex = 0; sectorIndex < pCommand->ioRequest.lbaRange.sectorCount; ++sectorIndex)
	{
		// modify verify data
		pLbaData[sectorIndex].isStatistic = false;

		// update verify backup
		pCommand->ioRequest.verifyLbaData[sectorIndex].isStatistic = true;
		pCommand->ioRequest.verifyLbaData[sectorIndex].writeCount ++;

		// init verify header
		verifyHeader.lba = startLba + sectorIndex;
		verifyHeader.verifyData = pCommand->ioRequest.verifyLbaData[sectorIndex].writeCount;
		// init write buffer
		memcpy(pCommand->ioRequest.buffer + cSectorSize * sectorIndex, &verifyHeader, sizeof(LbaVerifyHeader_t));
	}

}


static void VerifyWriteCompletedFunc(CommonCommand_t *pCommand)
{
	ASSERT_DEBUG(pCommand != NULL);
	ASSERT_DEBUG(pCommand->config.type == cCommandType_Io);
	ASSERT_DEBUG(pCommand->ioRequest.ioType == cIoType_Write);

	Disk_t *pDisk = pCommand->ioRequest.pDisk;
	uint64_t startLba = pCommand->ioRequest.lbaRange.startLba;
	int sectorCount = pCommand->ioRequest.lbaRange.sectorCount;

	switch (pCommand->config.status)
	{
		// write command not submit to disk
	case cCommandStatus_Abort:
	{
		LbaData_t *pLbaData = Disk_GetLbaVerifyDataAddr(pDisk, startLba);
		for (int sectorIndex = 0; sectorIndex < pCommand->ioRequest.lbaRange.sectorCount; ++sectorIndex)
		{
			pLbaData[sectorIndex].isStatistic = true;
		}
	}
	break;

	// failed and timeout lba status is not statistic
	case cCommandStatus_Failed:
	case cCommandStatus_Timeout:
		break;

	// write completed lba status is statistic, write count update
	case cCommandStatus_Success:
	{
		LbaData_t *pLbaData = Disk_GetLbaVerifyDataAddr(pDisk, startLba);
		memcpy((void *)(pLbaData), (void *)(pCommand->ioRequest.verifyLbaData), pCommand->ioRequest.lbaRange.sectorCount * sizeof(LbaData_t));
	}
	break;

	default:
		ASSERT(false);
	}
}

static void VerifyReadPrepareFunc(CommonCommand_t *pCommand)
{
	ASSERT_DEBUG(pCommand != NULL);
	ASSERT_DEBUG(pCommand->config.type == cCommandType_Io);
	ASSERT_DEBUG(pCommand->ioRequest.ioType == cIoType_Write);

	Disk_t *pDisk = pCommand->ioRequest.pDisk;
	uint64_t startLba = pCommand->ioRequest.lbaRange.startLba;
	int sectorCount = pCommand->ioRequest.lbaRange.sectorCount;

	LbaData_t *pLbaData = Disk_GetLbaVerifyDataAddr(pDisk, startLba);
	memcpy((void *)(pCommand->ioRequest.verifyLbaData), (void *)(pLbaData), pCommand->ioRequest.lbaRange.sectorCount * sizeof(LbaData_t));
}

static void VerifyReadCompletedFunc(CommonCommand_t *pCommand)
{
	ASSERT_DEBUG(pCommand != NULL);
	ASSERT_DEBUG(pCommand->config.type == cCommandType_Io);
	ASSERT_DEBUG(pCommand->ioRequest.ioType == cIoType_Write);

	Disk_t *pDisk = pCommand->ioRequest.pDisk;
	uint64_t startLba = pCommand->ioRequest.lbaRange.startLba;
	int sectorCount = pCommand->ioRequest.lbaRange.sectorCount;
	const LbaVerifyHeader_t *pVerifyHeader;

	LbaData_t *pLbaData = Disk_GetLbaVerifyDataAddr(pDisk, startLba);
	bool isVerifyFailed =  false;

	for (int sectorIndex = 0; sectorIndex < pCommand->ioRequest.lbaRange.sectorCount; ++sectorIndex)
	{
		pVerifyHeader = (LbaVerifyHeader_t *)(pCommand->ioRequest.buffer + cSectorSize * sectorIndex);
		switch (pCommand->ioRequest.verifyLbaData[sectorIndex].statue)
		{


		case cLbaStatue_NoInit:
			if ((pVerifyHeader->lba == startLba + sectorIndex) && (pVerifyHeader->verifyData <= cMaxVerifyCount))
			{
				pLbaData[sectorIndex].isStatistic = true;
				pLbaData[sectorIndex].writeCount = pVerifyHeader->verifyData;
				pLbaData[sectorIndex].statue = cLbaStatue_Valid;
			}
			else if (memcmp((void *)(pVerifyHeader), (void *)cZeroSector, cSectorSize) == 0)
			{
				pLbaData[sectorIndex].isStatistic = true;
				pLbaData[sectorIndex].writeCount = 0;
				pLbaData[sectorIndex].statue = cLbaStatue_Invalid;
			}
			break;

		case cLbaStatue_Invalid:
			if (pCommand->ioRequest.verifyLbaData[sectorIndex].isStatistic)
			{
				isVerifyFailed = memcpy((void *)(pVerifyHeader), (void *)cZeroSector, cSectorSize) != 0;
			}
			else
			{
				
			}
		break;

		case cLbaStatue_Valid:
		{
			isVerifyFailed = pVerifyHeader->lba != startLba + sectorIndex;
			if (!isVerifyFailed)
			{
				if (pCommand->ioRequest.verifyLbaData[sectorIndex].isStatistic)
				{
					isVerifyFailed = pVerifyHeader->verifyData == pVerifyHeader->verifyData;
				}
				else
				{
					pLbaData[sectorIndex].isStatistic = true;
					pLbaData[sectorIndex].writeCount = pVerifyHeader->verifyData;
				}
			}
		}

			break;
		
		default:
			ASSERT(false);
			break;
		}
		if (isVerifyFailed)
		{
			// TODO: print detail
			DebugPrint("verify failed sectorIndex: %d, lba: %d, writeCount: %d, statue: %d\n", sectorIndex, pVerifyHeader->lba, pVerifyHeader->verifyData, pLbaData[sectorIndex].statue);
			pCommand->config.status = cCommandStatus_VerifyFailed;
			isVerifyFailed = false;
		}
	}
}

//-----------------------------------------------------------------------------
//  Public functions
//-----------------------------------------------------------------------------

Disk_t *Disk_Create(const char *filePath, const char *verifyFilePath, uint64_t maxSectorCount)
{
	
	DebugPrint("Disk_Create: %s, %s\n", filePath, verifyFilePath);
	
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
		const char *pStart = strrchr(filePath, '/');
		if (pStart == NULL)
		{
			pStart = filePath;
		}
		else
		{
			pStart += 1;
		}

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
		Disk_VerifyFileDeInit(pDisk);
	}
	
	pthread_mutex_init(&pDisk->mutex, NULL);

	fprintf(stdout, "Disk create success\n");


	return pDisk;
ErrorOut:
	Disk_Destroy(pDisk);
	return NULL;
}


void Disk_VerifyFileDeInit(Disk_t *pDisk)
{
	if (pDisk == NULL)
	{
		return;
	}

	int pageNum = pDisk->verifyFileSize / getpagesize();

	LbaData_t *pLbaData = (LbaData_t *)(pDisk->pVerifyDataBaseAddr);

	for (int i = 0; i < getpagesize() / sizeof(LbaData_t); ++i)
	{
		pLbaData->all = cNoInitLbaData.all;
		pLbaData++;
	}

	void *pDest;
	for (int i = 1; i < pageNum; ++i)
	{
		pDest = pDisk->pVerifyDataBaseAddr + i * getpagesize();
		memcpy(pDest, (void *)(pDisk->pVerifyDataBaseAddr), getpagesize());
	}
	fprintf(stdout, "Verify file Deinit success\n");
}

void *Disk_GetLbaVerifyDataAddr(const Disk_t *pDisk, uint64_t lba)
{
	assert(lba < pDisk->maxSectorCount);
	if (pDisk == NULL || pDisk->pVerifyDataBaseAddr == MAP_FAILED)
	{
		return NULL;
	}
	return pDisk->pVerifyDataBaseAddr + lba * sizeof(LbaData_t);
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





