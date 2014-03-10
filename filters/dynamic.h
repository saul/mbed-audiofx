#ifndef _FILTER_DYNAMIC_H_
#define _FILTER_DYNAMIC_H_

typedef struct
{
	uint16_t sensitivity;
	uint16_t threshold;
} FilterNoiseGateData_t;


typedef struct
{
	uint16_t sensitivity;
	uint16_t threshold;
	float scalar;
} FilterCompressorData_t;


int32_t filter_noisegate_apply(int32_t input, void *pUnknown);
void filter_noisegate_debug(void *pUnknown);
void filter_noisegate_create(void *pUnknown);
int32_t filter_compressor_apply(int32_t input, void *pUnknown);
void filter_compressor_debug(void *pUnknown);
void filter_compressor_create(void *pUnknown);
int32_t filter_expander_apply(int32_t input, void *pUnknown);
void filter_expander_create(void *pUnknown);

#endif
