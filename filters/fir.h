#ifndef _FILTER_FIR_H_
#define _FILTER_FIR_H_

#pragma pack(push, 1)
typedef struct
{
	float *pflCoefficients;
	uint8_t nCoefficients;
} FilterFIRBaseData_t;
#pragma pack(pop)


#pragma pack(push, 1)
typedef struct
{
	FilterFIRBaseData_t base;
	uint16_t iLowerFreq;
	uint16_t iUpperFreq;
} FilterBandPassData_t;
#pragma pack(pop)


uint32_t filter_fir_apply(uint32_t input, void *pUnknown);
void filter_bandpass_debug(void *pUnknown);
void filter_bandpass_mod(void *pUnknown);

#endif
