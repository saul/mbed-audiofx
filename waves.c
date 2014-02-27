#include <math.h>
#include <stdint.h>

#include "dbg.h"
#include "samples.h"
#include "config.h"


uint8_t get_square(uint8_t frequency)
{
	if(g_iWaveCursor % (BUFFER_SAMPLES / frequency) > (BUFFER_SAMPLES / 2 / frequency))
		return 1;
	else
		return 0;
}


float get_sawtooth(uint8_t frequency)
{
	return fmod((float) g_iWaveCursor / BUFFER_SAMPLES * frequency, 1);
}


float get_inverse_sawtooth(uint8_t frequency)
{
	return 1 - get_sawtooth(frequency);
}


float get_triangle(uint8_t frequency)
{
	float calculation = fmod((float) g_iWaveCursor / BUFFER_SAMPLES * 2 * frequency, 2);

	if(calculation > 1)
		return 1 - (calculation - 1);

	return calculation;
}
