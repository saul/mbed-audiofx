#ifndef _FILTER_FLANGE_H_
#define _FILTER_FLANGE_H_

typedef struct
{
	uint16_t nDelay; // The baseline sample to use (nDelay in past). Must be < BUFFER_SAMPLES / 2
	uint8_t speed;
	uint8_t waveType; // 0 = Square, 1 = Sawtooth, 2 = Inverse Sawtooth, 3 = Triangle
	float originalMix;
	float flangedMix;
} FilterFlangeData_t;

uint32_t filter_flange_apply(uint32_5 input, void *pPrivate);
void filter_delay_debug(void *pPrivate);

#endif