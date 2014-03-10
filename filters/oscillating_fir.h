#ifndef _FILTER_OSCILLATING_FIR_H_
#define _FILTER_OSCILLATING_FIR_H_

#pragma pack(push, 1)
typedef struct
{
	FilterBandPassData_t bandPass;
	float flOscillatingFrequency;
	uint8_t waveType;	///< 0 = Square, 1 = Sawtooth, 2 = Inverse Sawtooth, 3 = Triangle
	uint16_t iMinFreq;
	uint16_t iMaxFreq;
} FilterOscillatingBandPassData_t;
#pragma pack(pop)


int32_t filter_oscillating_bandpass_apply(int32_t input, void *pUnknown);
void filter_oscillating_bandpass_debug(void *pUnknown);
void filter_oscillating_bandpass_mod(void *pUnknown);
void filter_oscillating_bandpass_create(void *pUnknown);

#endif
