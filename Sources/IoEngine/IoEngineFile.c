//-----------------------------------------------------------------------------
//  Include files:
//-----------------------------------------------------------------------------

#include "Type.h"
#include "IoEngine.h"
#include "CommonCommandPool.h"
#include "Disk.h"


#include <fcntl.h>

//-----------------------------------------------------------------------------
//  Constants definitions:
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
//  Macros definitions:
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
//  Data type definitions: typedef, struct or class
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
//  Private function proto-type definitions:
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
//  Data declaration: Private or Public
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
//  Imported data proto-type without header include
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
//  Imported function proto-type without header include
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
//  Private functions
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
//  Public functions
//-----------------------------------------------------------------------------
bool IoEngineFile_Init(IoEngine_t *pIoEngine)
{
	ASSERT(pIoEngine != NULL);
	return true;
}

bool IoEngineFile_CheckCompleted(IoEngine_t *pIoEngine)
{
	ASSERT(pIoEngine != NULL);
	return true;
}

bool IoEngineFile_Write(CommonCommand_t *pCommand)
{
	bool result = true;
	lseek(pCommand->ioRequest.pDisk->diskFileHandler, pCommand->ioRequest.lbaRange.startLba * cSectorSize, SEEK_SET);
	uint64_t size = pCommand->ioRequest.lbaRange.sectorCount * cSectorSize;
	
	result = write(pCommand->ioRequest.pDisk->diskFileHandler, pCommand->ioRequest.buffer, size) == size;
	
	pCommand->time.completeTime = time(NULL);
	pCommand->config.status = cCommandStatus_Success;
	if (!result)
	{
		pCommand->config.status = cCommandStatus_Failed;
	}

	return result;
}


bool IoEngineFile_Read(CommonCommand_t *pCommand)
{
	bool result = true;
	lseek(pCommand->ioRequest.pDisk->diskFileHandler, pCommand->ioRequest.lbaRange.startLba * cSectorSize, SEEK_SET);

	uint64_t size = pCommand->ioRequest.lbaRange.sectorCount * cSectorSize;
	result = read(pCommand->ioRequest.pDisk->diskFileHandler, pCommand->ioRequest.buffer, size) == size;
	
	pCommand->time.completeTime = time(NULL);
	pCommand->config.status = cCommandStatus_Success;
	if (!result)
	{
		pCommand->config.status = cCommandStatus_Failed;
	}
}

//-----------------------------------------------------------------------------
//  Private functions
//-----------------------------------------------------------------------------


