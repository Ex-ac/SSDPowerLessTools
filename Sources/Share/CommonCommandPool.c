//-----------------------------------------------------------------------------
//  Include files:
//-----------------------------------------------------------------------------
#include "CommonCommandPool.h"
#define _GNU_SOURCE

#include <pthread.h>

#include "Global.h"
#include "Debug.h"
#include "SimpleStack.h"
//-----------------------------------------------------------------------------
//  Constants definitions:
//-----------------------------------------------------------------------------
#if (COMMON_COMMAND_POOL_DEBUG)
#define _ASSERT_DEBUG(x)			ASSERT_DEBUG(x)
#else
#define _ASSERT_DEBUG(x)
#endif

//-----------------------------------------------------------------------------
//  Macros definitions:
//-----------------------------------------------------------------------------
SIMPLE_STACK_DEFINE_GEN(CommandId_t, CommandIdStack);
SIMPLE_STACK_IS_EMPTY_GEN(CommandId_t, CommandIdStack);
SIMPLE_STACK_IS_FULL_GEN(CommandId_t, CommandIdStack);
SIMPLE_STACK_POP_GEN(CommandId_t, CommandIdStack);
SIMPLE_STACK_PUSH_GEN(CommandId_t, CommandIdStack);
SIMPLE_STACK_GET_COUNT_GEN(CommandId_t, CommandIdStack);

//-----------------------------------------------------------------------------
//  Data type definitions: typedef, struct or class
//-----------------------------------------------------------------------------
typedef struct CommandPool
{
	pthread_mutex_t mutex;
	CommandIdStack_t commandIdStack;
	CommonCommand_t commandPool[cMaxCommonCommandPoolCount];
} CommandPool_t;

//-----------------------------------------------------------------------------
//  Private function proto-type definitions:
//-----------------------------------------------------------------------------
#if (COMMON_COMMAND_POOL_DEBUG)
static bool hasCommandId(CommandId_t commandId, CommandId_t pCommandIds[], unsigned int count);
#endif
//-----------------------------------------------------------------------------
//  Data declaration: Private or Public
//-----------------------------------------------------------------------------
static CommandPool_t sCommandPool;

//-----------------------------------------------------------------------------
//  Imported data proto-type without header include
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
//  Imported function proto-type without header include
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
//  Private functions
//-----------------------------------------------------------------------------
#if (COMMON_COMMAND_POOL_DEBUG)
static bool hasCommandId(CommandId_t commandId, CommandId_t pCommandIds[], unsigned int count)
{
	ASSERT(commandId < cMaxCommonCommandPoolCount);
	for (unsigned int i = 0; i < count; i++)
	{
		if (commandId == pCommandIds[i])
		{
			return true;
		}
	}
	return false;
}
#endif

//-----------------------------------------------------------------------------
//  Public functions
//-----------------------------------------------------------------------------

void CommonCommandPool_Init(void)
{
	pthread_mutex_init(&sCommandPool.mutex, NULL);
	pthread_mutex_lock(&sCommandPool.mutex);
	sCommandPool.commandIdStack.totalCount = cMaxCommonCommandPoolCount;
	sCommandPool.commandIdStack.insertIndex = 0;
	sCommandPool.commandIdStack.pData = (CommandId_t *)(malloc(cMaxCommonCommandPoolCount * sizeof(CommandId_t)));
	DebugPrint("sCommandPool.commandIdStack.pData = %p\n", sCommandPool.commandIdStack.pData);
	
	if (sCommandPool.commandIdStack.pData == NULL)
	{
		DebugPrint("sCommandPool.commandIdStack.pData = %p\n", sCommandPool.commandIdStack.pData);
		ASSERT(false);
	}

	for (unsigned int i = 0; i < cMaxCommonCommandPoolCount; i++)
	{
		sCommandPool.commandIdStack.pData[i] = i;
	}
	sCommandPool.commandIdStack.insertIndex = cMaxCommonCommandPoolCount;
	pthread_mutex_unlock(&sCommandPool.mutex);
}

void CommonCommandPool_DeInit(void)
{
	pthread_mutex_lock(&sCommandPool.mutex);
	ASSERT(CommandIdStack_IsFull(&sCommandPool.commandIdStack));
	free(sCommandPool.commandIdStack.pData);
	pthread_mutex_unlock(&sCommandPool.mutex);

	pthread_mutex_destroy(&sCommandPool.mutex);
}

// bool CommonCommandPool_TryLock(void);
// void CommonCommandPool_Unlock(void);

CommandId_t CommonCommandPool_Alloc(void)
{
	CommandId_t ret = cInvalidCommandId;
	pthread_mutex_lock(&sCommandPool.mutex);
	CommandIdStack_Pop(&sCommandPool.commandIdStack, &ret);
	pthread_mutex_unlock(&sCommandPool.mutex);
	return ret;
}

void CommonCommandPool_Dealloc(CommandId_t commandId)
{
	pthread_mutex_lock(&sCommandPool.mutex);
	_ASSERT_DEBUG(!hasCommandId(commandId, (CommandId_t *)(sCommandPool.commandIdStack.pData), sCommandPool.commandIdStack.insertIndex));

	bool ok = CommandIdStack_Push(&sCommandPool.commandIdStack, commandId);
	ASSERT(ok);
	pthread_mutex_unlock(&sCommandPool.mutex);
}

bool CommonCommandPool_Allocs(unsigned int count, CommandId_t pCommandIds[])
{
	pthread_mutex_lock(&sCommandPool.mutex);
	if (count > CommandIdStack_GetCount(&sCommandPool.commandIdStack))
	{
		pthread_mutex_unlock(&sCommandPool.mutex);
		return false;
	}
	memcpy(pCommandIds, sCommandPool.commandIdStack.pData + sCommandPool.commandIdStack.insertIndex - count, count * sizeof(CommandId_t));
	sCommandPool.commandIdStack.insertIndex -= count;
	pthread_mutex_unlock(&sCommandPool.mutex);
	return true;
}

void CommonCommandPool_Deallocs(unsigned int count, CommandId_t pCommandIds[])
{
	pthread_mutex_lock(&sCommandPool.mutex);
	ASSERT(sCommandPool.commandIdStack.insertIndex + count <= sCommandPool.commandIdStack.totalCount);

	
	for (unsigned int i = 0; i < (count - 1); i++)
	{
		_ASSERT_DEBUG(!hasCommandId(pCommandIds[i], pCommandIds + i + 1, count - i - 1));
	}

	for (unsigned int i = 0; i < count; i++)
	{
		_ASSERT_DEBUG(!hasCommandId(pCommandIds[i], sCommandPool.commandIdStack.pData, sCommandPool.commandIdStack.insertIndex));
	}



	memcpy(sCommandPool.commandIdStack.pData + sCommandPool.commandIdStack.insertIndex, pCommandIds, count * sizeof(CommandId_t));
	sCommandPool.commandIdStack.insertIndex += count;
	pthread_mutex_unlock(&sCommandPool.mutex);
}

unsigned int CommonCommandPool_GetSize(void)
{
	pthread_mutex_lock(&sCommandPool.mutex);
	unsigned int ret = CommandIdStack_GetCount(&sCommandPool.commandIdStack);
	pthread_mutex_unlock(&sCommandPool.mutex);
	return ret;
}

CommonCommand_t *CommonCommandPool_GetCommand(CommandId_t commandId)
{
	ASSERT(commandId < cMaxCommonCommandPoolCount);
	return sCommandPool.commandPool + commandId;
}