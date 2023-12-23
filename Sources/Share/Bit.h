#pragma once
#include "Type.h"

#ifdef __cplusplus
extern "C" {
#endif

#define BIT(x)			(0x01 << x)
#define BIT_MASK(x)		(BIT(x) - 1)
#define BIT_CLR(x, b)	(x = __BIC(x, b))


#pragma always_inline
uint_t __BIC(uint_t x, uint_t bit)
{
	return (x & (~BIT(bit)));
}



#pragma always_inline
uint_t Bit_CountLeadingZero(uint_t data)
{
	return __builtin_clz(data);
}

__inline static uint_t Bit_CountTailZero(uint_t data)
{
	return __builtin_ctz(data);
}

#pragma always_inline
uint_t Bit_CountLeadingOne(uint_t data)
{
	return __builtin_clz(~data);
}

#pragma always_inline
uint_t Bit_CountTailOne(uint_t data)
{
	return __builtin_ctz(~data);
}


#pragma always_inline
uint_t Bit_CountBit(uint_t x)
{
	uint_t xx = x;
	xx = xx - ((xx >> 1) & 0x55555555);
	xx = (xx & 0x33333333) + ((xx >> 2) & 0x33333333);
	xx = (xx + (xx >> 4)) & 0x0f0f0f0f;
	xx = xx + (xx >> 8);
	return (xx + (xx >> 16)) & 0xff;
}


#pragma always_inline
uint_t Bit_GetBitmap(const uint_t *pData, uint_t offset)
{
	uint_t ret = offset / 8;
	offset &= 0x7;
	pData = ((uint_t *)((uint8_t *)(pData)) + ret);
	ret = *pData >> offset;
	return ret;
}

#ifdef __cplusplus
}
#endif