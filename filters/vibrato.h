#ifndef _FILTER_VIBRATO_H_
#define _FILTER_VIBRATO_H_

typedef struct
{
	uint16_t nDelay; ///< The baseline sample to use (nDelay in past). Must be < BUFFER_SAMPLES / 2
	uint8_t frequency;
	uint8_t waveType; ///< 0 = Square, 1 = Sawtooth, 2 = Inverse Sawtooth, 3 = Triangle
} FilterFlangeData_t;

#endif
