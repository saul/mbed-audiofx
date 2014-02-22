#ifndef _FILTER_TREMOLO_H_
#define _FILTER_TREMOLO_H_

typedef struct
{
	uint8_t frequency;	///< (Divisor of SAMPLE_RATE)/4
	uint8_t waveType;	///< 0 = Square, 1 = Sawtooth, 2 = Inverse Sawtooth, 3 = Triangle
	float depth;		///< Float f; 0 <= f <= 1 (0 = silenct, 1 = original signal)
} FilterTremoloData_t;

uint32_t filter_tremolo_apply(uint32_t input, void *pPrivate);

#endif
