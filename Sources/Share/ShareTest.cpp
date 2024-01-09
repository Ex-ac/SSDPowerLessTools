#include <gtest/gtest.h>

#include "CommonCommandPool.h"
TEST(CommonCommandPool, BaseTest)
{
	CommandId_t ids[cMaxCommonCommandPoolCount];
	CommonCommandPool_Init();
	ASSERT_EQ(CommonCommandPool_GetSize(), cMaxCommonCommandPoolCount);

	CommonCommand_t *pStartCommand = CommonCommandPool_GetCommand(0);
	CommonCommand_t *pCommand = NULL;
	for (int i = 0; i < cMaxCommonCommandPoolCount; ++i)
	{
		pCommand = CommonCommandPool_GetCommand(i);
		ASSERT_EQ(pCommand - pStartCommand, i) << "i = " << i << ", pCommand = " << pCommand << ", pStartCommand = " << pStartCommand << std::endl;
	}

	CommandId_t id = 0;
	for (int i = 0; i < cMaxCommonCommandPoolCount; ++i)
	{
		id = CommonCommandPool_Alloc();
		ASSERT_EQ(id, cMaxCommonCommandPoolCount - i - 1);
	}

	ASSERT_EQ(CommonCommandPool_GetSize(), 0);
	ASSERT_EQ(CommonCommandPool_Alloc(), cInvalidCommandId);

	for (int i = 0; i < cMaxCommonCommandPoolCount; ++i)
	{
		CommonCommandPool_Dealloc(cMaxCommonCommandPoolCount - i - 1);
	}

	ASSERT_EQ(CommonCommandPool_GetSize(), cMaxCommonCommandPoolCount);

	CommonCommandPool_Allocs(cMaxCommonCommandPoolCount, ids);
	ASSERT_EQ(CommonCommandPool_GetSize(), 0);

	for (int i = 0; i < cMaxCommonCommandPoolCount; ++i)
	{
		ASSERT_EQ(ids[i], cMaxCommonCommandPoolCount - i - 1);
	}

	CommonCommandPool_Deallocs(cMaxCommonCommandPoolCount, ids);
	ASSERT_EQ(CommonCommandPool_GetSize(), cMaxCommonCommandPoolCount);

	CommonCommandPool_DeInit();
}