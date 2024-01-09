#include <gtest/gtest.h>

#include <random>
#include <ctime>

#include "IoEngine.h"
#include "Debug.h"
#include "CommonCommandPool.h"


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

TEST(IoEngineTest, IoRequest)
{
	std:srand(std::time(0));

	uint_t completedCount = 0;

	CommonCommandPool_Init();
	const int requestCount = 32;
	DebugPrint("IoEngineTest::IoRequest\n");
	IoEngine_t *pIoEngine = IoEngine_Create(requestCount);
	ASSERT_NE((void *)(pIoEngine), nullptr);

	IoEngine_Run(pIoEngine);
	
	ASSERT_EQ(IoEngine_RequestQueueFreeCount(pIoEngine), requestCount - 1);
	
	CommandId_t completedCommandId = cInvalidCommandId;
	
	for (int i = 0; i < 1024 * 1024; i++)
	{
		CommandId_t commandId = CommonCommandPool_Alloc();

		while (commandId == cInvalidCommandId)
		{
			DebugPrint("CommonCommandPool_Alloc failed %d\n", i);
			uint_t count = IoEngine_CompletedQueueCount(pIoEngine);

			while (count --)
			{
				bool ok = IoEngine_CompletedQueuePop(pIoEngine, &completedCommandId);
				ASSERT_EQ(ok, true);
				ASSERT_NE(completedCommandId, cInvalidCommandId);

				completedCount ++;
				DebugPrint("CommonCommandPool_Dealloc %d\n", completedCommandId);
				CommonCommandPool_Dealloc(completedCommandId);

			}
			commandId = CommonCommandPool_Alloc();
		}
		
		DebugPrint("CommonCommandPool_Alloc %d\n", commandId);
		DebugPrint("IoEngine_Submit %d\n", i);

		CommonCommand_t *pCommand = CommonCommandPool_GetCommand(commandId);
		pCommand->config.commandId = i;
		pCommand->time.startTime = clock();
		pCommand->time.timeoutMs = std::rand() % 1000;


		while (IoEngine_Submit(pIoEngine, commandId) == false)
		{
			DebugPrint("IoEngine_Submit failed %d\n", i);
			usleep(1000 * std::rand() % 10);
		}
	}

	while (completedCount != 1024)
	{
		uint_t count = IoEngine_CompletedQueueCount(pIoEngine);

		while (count--)
		{
			bool ok = IoEngine_CompletedQueuePop(pIoEngine, &completedCommandId);
			ASSERT_EQ(ok, true);
			ASSERT_NE(completedCommandId, cInvalidCommandId);

			DebugPrint("CommonCommandPool_Dealloc %d\n", completedCommandId);
			CommonCommandPool_Dealloc(completedCommandId);
			completedCount ++;
		}
	}

	IoEngine_Destroy(pIoEngine);
	CommonCommandPool_DeInit();
}