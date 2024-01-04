
//-----------------------------------------------------------------------------
//  Include files:
//-----------------------------------------------------------------------------
#include "SimpleFifo.h"
#include "Bit.h"

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

static bool EntryNumCheck(uint_t entryNum);

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

static bool EntryNumCheck(uint_t entryNum)
{
	int leftOneNum = Bit_CountTailZero(entryNum);
	entryNum >>= (leftOneNum + 1);
	DebugPrint("entryNum = %d, leftOneNum = %d", entryNum, leftOneNum);
	return entryNum == 0x00;
}

//-----------------------------------------------------------------------------
//  Public functions
//-----------------------------------------------------------------------------


void SimpleFifo_Init(SimpleFifo_t *pFifo, uint_t entryNum, uint_t entrySize, void *pEntry)
{
	ASSERT(EntryNumCheck(entryNum));
	pFifo->entrySize = entrySize;
	pFifo->mask = entryNum - 1;
	pFifo->pEntry = pEntry;
	pFifo->frontIndex = 0;
	pFifo->rearIndex = 0;
}

SimpleFifo_t *SimpleFifo_Create(uint_t entryNum, uint_t entrySize)
{
	ASSERT(EntryNumCheck(entryNum));
	SimpleFifo_t *pFifo = (SimpleFifo_t *)malloc(sizeof(SimpleFifo_t));
	if (pFifo != NULL)
	{
		memset(pFifo, 0, sizeof(SimpleFifo_t));
		pFifo->entrySize = entrySize;
		pFifo->mask = entryNum - 1;
		pFifo->pEntry = malloc(entryNum * entrySize);
		if (pFifo->pEntry == NULL)
		{
			free(pFifo);
			pFifo = NULL;
		}
	}
	return pFifo;
}

void SimpleFifo_Destroy(SimpleFifo_t *pFifo)
{
	free(pFifo->pEntry);
	free(pFifo);
}