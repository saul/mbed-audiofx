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
	uint16_t iCentreFreq;
	uint16_t iWidth;
} FilterBandPassData_t;
#pragma pack(pop)


int16_t filter_fir_apply(int16_t input, void *pUnknown);
void filter_bandpass_debug(void *pUnknown);
void filter_bandpass_mod(void *pUnknown);
void filter_bandpass_create(void *pUnknown);

#endif
