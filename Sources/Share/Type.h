#pragma once


#ifdef __cplusplus
extern "C" {
#endif


//-----------------------------------------------------------------------------
//  Include files:
//-----------------------------------------------------------------------------
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <assert.h>
#include <unistd.h>
#include <time.h>

#include "SimpleFifo.h"

// WARNING: don't include Global.h

//-----------------------------------------------------------------------------
//  Constant definitions:
//-----------------------------------------------------------------------------
#define MAX_COMMON_COMMAND_SIZE		0x40
#define cInvalidCommandId			__UINT32_MAX__
#define cSectorSize					512

//-----------------------------------------------------------------------------
//  Macros definitions:
//-----------------------------------------------------------------------------
#ifndef OFFSET_OF
#define OFFSET_OF(TYPE, MEMBER) ((size_t) &((TYPE*)0)->MEMBER)
#endif

#ifndef CONTAINER_OF
#define CONTAINER_OF(PTR, TYPE, MEMBER) ({          \
        const typeof(((TYPE *)0)->MEMBER)*__mptr = (PTR);    \
    (TYPE *)((char *)__mptr - offsetof(TYPE, MEMBER)); })
#endif



//-----------------------------------------------------------------------------
//  Data type definitions: typedef, struct or class
//-----------------------------------------------------------------------------
struct Disk;


typedef unsigned int uint_t;
typedef unsigned int CommandId_t;
typedef enum CommandType
{
	cCommandType_Io = 0,
	cCommandType_Admin,
	cCommandType_Invalid,
} CommandType_t;
typedef union LbaRange
{
	uint64_t all;
	struct
	{
		uint64_t startLba : 48;
		uint64_t sectorCount : 16;
	};
} LbaRange_t;

typedef enum IoType
{
	cIoType_Write = 0,
	cIoType_Read,
	cMaxNumberOfIOType,
} IoType_t;


typedef enum CommandStatus
{
	cCommandStatus_Success = 0,
	cCommandStatus_Failed,
	cCommandStatus_Abort,
	cCommandStatus_Timeout,
	cCommandStatus_Submit,
	cCommandStatus_Invalid,
} CommandStatus_t;

typedef struct IoRequest
{
	struct Disk *pDisk;
	IoType_t ioType;
	LbaRange_t lbaRange;
	void *buffer;
} IoRequest_t;


typedef union CommandCommandConfig
{
	uint64_t all;
	struct 
	{
		CommandType_t type		: 8;
		uint8_t commandId;
		CommandStatus_t status	: 8;
		uint64_t reserved		: 40;
	};
} CommandCommandConfig_t;

typedef struct CommandTime
{
	clock_t startTime;		// command generator
	clock_t submitTime;		// command submit to device
	clock_t completeTime;	// command complete
	int timeoutMs;			// command timeout
} CommandTime_t;

typedef struct CommonCommand
{
	CommandCommandConfig_t config;
	CommandTime_t time;
	union
	{
		uint8_t all[MAX_COMMON_COMMAND_SIZE];
		struct
		{
			IoRequest_t ioRequest;
			uint32_t ioReserved[MAX_COMMON_COMMAND_SIZE - sizeof(IoRequest_t)];
		};
	};
} CommonCommand_t;

typedef struct CommandPack
{
	uint_t count;
	CommonCommand_t command[];
} CommandPack_t;

SIMPLE_FIFO_TYPE_DEFINE_GEN(CommandId_t, CommandIdFifo);
SIMPLE_FIFO_TYPE_INIT_GEN(CommandId_t, CommandIdFifo);
SIMPLE_FIFO_TYPE_COUNT_GEN(CommandId_t, CommandIdFifo);
SIMPLE_FIFO_TYPE_FREE_COUNT_GEN(CommandId_t, CommandIdFifo);
SIMPLE_FIFO_TYPE_IS_FULL_GEN(CommandId_t, CommandIdFifo);
SIMPLE_FIFO_TYPE_IS_EMPTY_GEN(CommandId_t, CommandIdFifo);
SIMPLE_FIFO_TYPE_POP_GEN(CommandId_t, CommandIdFifo);
SIMPLE_FIFO_TYPE_PUSH_GEN(CommandId_t, CommandIdFifo);


//-----------------------------------------------------------------------------
//  Public interface functions:
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
//  Inline functions
//-----------------------------------------------------------------------------


#ifdef __cplusplus
}
#endif