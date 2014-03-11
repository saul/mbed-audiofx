#ifndef _FILTER_VIBRATO_H_
#define _FILTER_VIBRATO_H_

#include <stdbool.h>

typedef struct
{
	uint16_t nDelay; ///< The baseline sample to use (nDelay in past). Must be < BUFFER_SAMPLES / 2
	uint8_t frequency;
	uint8_t waveType; ///< 0 = Square, 1 = Sawtooth, 2 = Inverse Sawtooth, 3 = Triangle
} FilterVibratoData_t;


extern volatile bool g_bVibratoActive;


float vibrato_get_cursor(void *pUnknown);
int16_t filter_vibrato_apply(int16_t input, void *pUnknown);
void filter_vibrato_debug(void *pUnknown);
void filter_vibrato_create(void *pUnknown);

#endif
