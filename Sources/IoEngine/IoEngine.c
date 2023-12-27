#include "IoEngine.h"


#ifdef Uint
#undef Uint
#endif

// using the unsigned int to save addr
#define Uint			uint_t

SIMPLE_FIFO_TYPE_DEFINE_GEN(Uint);
SIMPLE_FIFO_TYPE_INIT_GEN(Uint);
SIMPLE_FIFO_TYPE_COUNT_GEN(Uint);
SIMPLE_FIFO_TYPE_IS_FULL_GEN(Uint);
SIMPLE_FIFO_TYPE_IS_EMPTY_GEN(Uint);
SIMPLE_FIFO_TYPE_GET_GEN(Uint);
SIMPLE_FIFO_TYPE_PUT_GEN(Uint);


IoEngine_t *IoEngine_Create(uint_t ioQueueDepth)
{

}


void IoEngine_Run(IoEngine_t *pIoEngine);
void IoEngine_Destroy(IoEngine_t *pIoEngine);