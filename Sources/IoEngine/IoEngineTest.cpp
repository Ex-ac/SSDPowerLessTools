#include <gtest/gtest.h>

#include <random>
#include <ctime>

#include "IoEngine.h"
#include "Debug.h"
#include "CommonCommandPool.h"
#include "Disk.h"


TEST(IoEngineTest, CreateAndDestroy)
{
	DebugPrint("IoEngineTest::Create&Destroy\n");
	for (int i = 0; i < 10; i++)
	{
		printf("i = %d\n", i);


		IoEngine_t *pIoEngine = IoEngine_Create(1024*1024);
		ASSERT_NE((void *)(pIoEngine), nullptr);
		// sleep(1);

		if (pIoEngine != NULL)
		{
			printf("Create success\n");
			printf("Now destroy it\n");
			IoEngine_Destroy(pIoEngine);
		}
	}
}

// TEST(IoEngineTest, IoRequest)
// {
// 	std:srand(std::time(0));

// 	uint_t completedCount = 0;
// 	const int testCount = 1024 * 1024;

// 	CommonCommandPool_Init();
// 	const int requestCount = 32;
// 	DebugPrint("IoEngineTest::IoRequest\n");
// 	IoEngine_t *pIoEngine = IoEngine_Create(requestCount);
// 	ASSERT_NE((void *)(pIoEngine), nullptr);

// 	IoEngine_Run(pIoEngine);
	
// 	ASSERT_EQ(IoEngine_RequestQueueFreeCount(pIoEngine), requestCount - 1);
	
// 	CommandId_t completedCommandId = cInvalidCommandId;
	
// 	for (int i = 0; i < testCount; i++)
// 	{
// 		CommandId_t commandId = CommonCommandPool_Alloc();

// 		while (commandId == cInvalidCommandId)
// 		{
// 			DebugPrint("CommonCommandPool_Alloc failed %d\n", i);
// 			uint_t count = IoEngine_CompletedQueueCount(pIoEngine);

// 			while (count --)
// 			{
// 				bool ok = IoEngine_CompletedQueuePop(pIoEngine, &completedCommandId);
// 				ASSERT_EQ(ok, true);
// 				ASSERT_NE(completedCommandId, cInvalidCommandId);

// 				completedCount ++;
// 				DebugPrint("CommonCommandPool_Dealloc %d\n", completedCommandId);
// 				CommonCommandPool_Dealloc(completedCommandId);

// 			}
// 			commandId = CommonCommandPool_Alloc();
// 		}
		
// 		DebugPrint("CommonCommandPool_Alloc %d\n", commandId);
// 		DebugPrint("IoEngine_Submit %d\n", i);

// 		CommonCommand_t *pCommand = CommonCommandPool_GetCommand(commandId);
// 		pCommand->config.commandId = i;
// 		pCommand->time.startTime = clock();
// 		pCommand->time.timeoutMs = std::rand() % 100;


// 		while (IoEngine_Submit(pIoEngine, commandId) == false)
// 		{
// 			DebugPrint("IoEngine_Submit failed %d\n", i);
// 			usleep(1000  * std::rand() % 10);
// 		}
// 	}

// 	while (completedCount != testCount)
// 	{
// 		uint_t count = IoEngine_CompletedQueueCount(pIoEngine);

// 		while (count--)
// 		{
// 			bool ok = IoEngine_CompletedQueuePop(pIoEngine, &completedCommandId);
// 			ASSERT_EQ(ok, true);
// 			ASSERT_NE(completedCommandId, cInvalidCommandId);

// 			DebugPrint("CommonCommandPool_Dealloc %d\n", completedCommandId);
// 			CommonCommandPool_Dealloc(completedCommandId);
// 			completedCount ++;
// 		}
// 	}

// 	IoEngine_Destroy(pIoEngine);
// 	CommonCommandPool_DeInit();
// }


#define cTestDiskSize (1024 * 1024 * 1024)
#define cBufferSize (128 * 1024)

TEST(IoEngineTest, BaseWriteAndRead)
{
	Disk_t *pDisk = Disk_Create("./test.bin", NULL, cTestDiskSize / cSectorSize);
	ASSERT_NE((void *)(pDisk), nullptr);
	uint8_t (*bufferPool)[cBufferSize];
	
	if (posix_memalign((void **)(&bufferPool), cBufferSize, cBufferSize * cMaxCommonCommandPoolCount))
	{
		ASSERT_FALSE(true);
		DebugPrint("Alloc buffer pool failed");
		return;
	}

	CommonCommandPool_Init();

	IoEngine_t *pIoEngine = IoEngine_Create(cMaxCommonCommandPoolCount);
	ASSERT_NE((void *)(pIoEngine), nullptr);

	IoEngine_Run(pIoEngine);

	uint64_t sectorCount = cBufferSize / cSectorSize;

	CommandId_t commandId;
	CommandId_t completedCommandId = cInvalidCommandId;

	uint64_t completedCount = 0;
	uint64_t submitCount = 0;

	for (uint64_t sectorOffset = 0; sectorOffset < pDisk->maxSectorCount; sectorOffset += sectorCount)
	{

		commandId = CommonCommandPool_Alloc();

		while (commandId == cInvalidCommandId)
		{
			uint_t count = IoEngine_CompletedQueueCount(pIoEngine);

			while (count--)
			{
				bool ok = IoEngine_CompletedQueuePop(pIoEngine, &completedCommandId);
				ASSERT_EQ(ok, true);
				ASSERT_NE(completedCommandId, cInvalidCommandId);

				completedCount++;
				DebugPrint("CommonCommandPool_Dealloc %d\n", completedCommandId);
				CommonCommandPool_Dealloc(completedCommandId);
			}
			commandId = CommonCommandPool_Alloc();
		}

		DebugPrint("CommonCommandPool_Alloc %d\n", commandId);
		DebugPrint("IoEngine_Submit %ld\n", submitCount);

		CommonCommand_t *pCommand = CommonCommandPool_GetCommand(commandId);
		pCommand->config.status = cCommandStatus_Success;
		pCommand->config.commandId = submitCount;
		pCommand->time.startTime = clock();
		pCommand->time.timeoutMs = 100;

		pCommand->ioRequest.ioType = cIoType_Write;
		pCommand->ioRequest.pDisk = pDisk;
		pCommand->ioRequest.lbaRange = {
			.startLba = sectorOffset,
			.sectorCount = sectorCount,
		};

		void *pLbaData = Disk_GetLbaVerifyDataAddr(pDisk, sectorOffset);
		memcpy((void *)(pCommand->ioRequest.verifyLbaData), pLbaData, sizeof(LbaData_t) * sectorCount);
		for (uint64_t i = 0; i < sectorCount; i++)
		{
			pCommand->ioRequest.verifyLbaData[i].isStatistic = false;
		}
		memcpy


		pCommand->ioRequest.buffer = bufferPool + commandId;
		DebugPrint("CommandId: %d, buffer addr: %p", commandId, pCommand->ioRequest.buffer);

		while (IoEngine_Submit(pIoEngine, commandId) == false)
		{
			usleep(1000 * std::rand() % 10);
		}

		submitCount++;
	}

	while (completedCount != submitCount)
	{
		uint_t count = IoEngine_CompletedQueueCount(pIoEngine);

		while (count--)
		{
			bool ok = IoEngine_CompletedQueuePop(pIoEngine, &completedCommandId);
			ASSERT_EQ(ok, true);
			ASSERT_NE(completedCommandId, cInvalidCommandId);

			DebugPrint("CommonCommandPool_Dealloc %d\n", completedCommandId);
			CommonCommandPool_Dealloc(completedCommandId);
			completedCount++;
		}
	}

	IoEngine_Destroy(pIoEngine);
	CommonCommandPool_DeInit();
	free(bufferPool);
}



