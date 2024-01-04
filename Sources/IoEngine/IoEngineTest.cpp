#include <gtest/gtest.h>

#include "IoEngine.h"
#include "Debug.h"


TEST(IoEngineTest, CreateAndDestroy)
{
	DebugPrint("IoEngineTest::Create&Destroy\n");
	for (int i = 0; i < 10; i++)
	{
		printf("i = %d\n", i);


		IoEngine_t *pIoEngine = IoEngine_Create(1024 * 1024 * 1024);
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
	const int requestCount = 32;
	DebugPrint("IoEngineTest::IoRequest\n");
	IoEngine_t *pIoEngine = IoEngine_Create(requestCount);
	ASSERT_NE((void *)(pIoEngine), nullptr);

	ASSERT_EQ(IoEngine_RequestQueueFreeCount(pIoEngine), requestCount - 1);
	CommonCommand_t command;
	for (int i = 0; i < 1024; i++)
	{
		command.config.commandId = i;
		DebugPrint("IoEngine_Submit %d\n", i);
		while (IoEngine_Submit(pIoEngine, &command) == false)
		{
			DebugPrint("IoEngine_Submit failed %d\n", i);
			sleep(1);
		}
	}

	IoEngine_Destroy(pIoEngine);
}