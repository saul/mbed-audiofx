#ifndef _FILTER_TREMOLO_H_
#define _FILTER_TREMOLO_H_

typedef struct
{
	uint8_t frequency;	///< (Divisor of SAMPLE_RATE)/4
	uint8_t waveType;	///< 0 = Square, 1 = Sawtooth, 2 = Inverse Sawtooth, 3 = Triangle
	float depth;		///< Float f; 0 <= f <= 1 (0 = silence, 1 = original signal)
} FilterTremoloData_t;

int16_t filter_tremolo_apply(int16_t input, void *pUnknown);
void filter_tremolo_debug(void *pUnknown);
void filter_tremolo_create(void *pUnknown);

#endif
