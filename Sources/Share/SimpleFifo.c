#include "SimpleFifo.h"
#include "Bit.h"


static bool EntryNumCheck(uint_t entryNum);

void SimpleFifo_Init(SimpleFifo_t *pFifo, uint_t entryNum, uint_t entrySize, void *pEntry)
{
	ASSERT(EntryNumCheck(entryNum));
	pFifo->entrySize = entrySize;
	pFifo->mask = entryNum - 1;
	pFifo->pEntry = pEntry;
	pFifo->frontIndex = 0;
	pFifo->rearIndex = 0;
}



static bool EntryNumCheck(uint_t entryNum)
{
	int leftOneNum = Bit_CountTailZero(entryNum);
	entryNum >= (leftOneNum + 1);
	return entryNum == 0x00;
}