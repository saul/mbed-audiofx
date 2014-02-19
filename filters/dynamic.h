#ifndef _FILTER_DYNAMIC_H_
#define _FILTER_DYNAMIC_H_

typedef struct
{
	uint16_t sensitivity;
	uint16_t threshold;
} FilterNoiseGateData_t;


uint32_t filter_noisegate_apply(uint32_t input, void *pPrivate);
void filter_noisegate_debug(void *pPrivate);

#endif
