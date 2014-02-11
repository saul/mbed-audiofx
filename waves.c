#include <math.h>
#include <stdint.h>

#include "dbg.h"
#include "samples.h"
#include "config.h"


uint8_t get_square(uint8_t speed)
{
	if(g_iSampleCursor % (BUFFER_SAMPLES / speed) > (BUFFER_SAMPLES / 2 / speed))
		return 1;
	else
		return 0;
}


float get_sawtooth(uint8_t speed)
{
	return fmod((float) g_iSampleCursor / BUFFER_SAMPLES * speed, 1);
}


float get_inverse_sawtooth(uint8_t speed)
{
	return 1 - get_sawtooth(speed);
}


float get_triangle(uint8_t speed)
{
	float calculation = fmod((float) g_iSampleCursor / BUFFER_SAMPLES * 2 * speed, 2);

	if(calculation > 1)
		return 1 - (calculation - 1);

	return calculation;
}
