#ifndef _FILTER_FLANGE_H_
#define _FILTER_FLANGE_H_

typedef struct
{
	uint16_t nDelay; ///< The baseline sample to use (nDelay in past). Must be < BUFFER_SAMPLES / 2
	uint8_t frequency;
	uint8_t waveType; ///< 0 = Square, 1 = Sawtooth, 2 = Inverse Sawtooth, 3 = Triangle
	float flangedMix;
} FilterFlangeData_t;


int16_t filter_flange_apply(int16_t input, void *pUnknown);
void filter_flange_debug(void *pUnknown);
void filter_flange_create(void *pUnknown);

#endif
