#ifndef _FILTER_DISTORTION_H_
#define _FILTER_DISTORTION_H_

typedef struct
{
	uint8_t bitLoss;	// between 0 and 10: number of bits of sensitivity to lose
} FilterBitcrusherData_t;


uint32_t filter_bitcrusher_apply(uint32_t input, void *pUnknown);
void filter_bitcrusher_debug(void *pUnknown);
void filter_bitcrusher_create(void *pUnknown);

#endif
