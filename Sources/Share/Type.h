#pragma once


#ifdef __cplusplus
extern "C" {
#endif


//-----------------------------------------------------------------------------
//  Include files:
//-----------------------------------------------------------------------------
#include <stdbool.h>
#include <stdint.h>
#include <assert.h>

//-----------------------------------------------------------------------------
//  Constant definitions:
//-----------------------------------------------------------------------------
#define MAX_COMMON_COMMAND_SIZE		0x40

//-----------------------------------------------------------------------------
//  Macros definitions:
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
//  Data type definitions: typedef, struct or class
//-----------------------------------------------------------------------------
struct Disk_t;


typedef unsigned int uint_t;

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
} IoType_t;


typedef struct IoRequest
{
	struct Disk_t *pDisk;
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
		uint64_t reserved		: 48;
	};
	
} CommandCommandConfig_t;
typedef struct CommonCommand
{
	CommandCommandConfig_t config;
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


//-----------------------------------------------------------------------------
//  Public interface functions:
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
//  Inline functions
//-----------------------------------------------------------------------------


#ifdef __cplusplus
}
#endif