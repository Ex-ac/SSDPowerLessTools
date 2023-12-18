#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>
#include <assert.h>

#define ASSERT(x)			assert(x)


typedef union LbaRange
{
	uint64_t all;
	struct
	{
		uint64_t startLba : 48;
		uint64_t sectorCount : 16;
	};
} LbaRange_t;





#ifdef __cplusplus
}
#endif