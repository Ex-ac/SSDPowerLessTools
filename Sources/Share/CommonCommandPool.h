#pragma once


#ifdef __cplusplus
extern "C" {
#endif


//-----------------------------------------------------------------------------
//  Include files:
//-----------------------------------------------------------------------------
#include "Type.h"

//-----------------------------------------------------------------------------
//  Constant definitions:
//-----------------------------------------------------------------------------
#define cMaxCommonCommandPoolCount				1024
#define COMMON_COMMAND_POOL_DEBUG				true
//-----------------------------------------------------------------------------
//  Macros definitions:
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
//  Data type definitions: typedef, struct or class
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
//  Public interface functions:
//-----------------------------------------------------------------------------

void CommonCommandPool_Init(void);
void CommonCommandPool_DeInit(void);

// bool CommonCommandPool_TryLock(void);
// void CommonCommandPool_Unlock(void);

CommandId_t CommonCommandPool_Alloc(void);
void CommonCommandPool_Dealloc(CommandId_t commandId);

bool CommandCommandPool_Allocs(unsigned int count, CommandId_t pCommandIds[]);
void CommandCommandPool_Deallocs(unsigned int count, CommandId_t pCommandIds[]);

unsigned int CommonCommandPool_GetSize(void);
CommonCommand_t *CommonCommandPool_GetCommand(CommandId_t commandId);

//-----------------------------------------------------------------------------
//  Inline functions
//-----------------------------------------------------------------------------


#ifdef __cplusplus
}
#endif