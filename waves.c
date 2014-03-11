/*
 *	HAPR Project 2014
 *	Group 6 - Tom Bryant (TB) & Saul Rennison (SR)
 *
 *	File created by:	TB
 *	File modified by:	TB
 *	File debugged by:	TB
 */


#include <math.h>
#include <stdint.h>

#include "dbg.h"
#include "samples.h"
#include "config.h"


/*
 *	Returns the value of a square wave at the current
 *	point in time, and at the requested frequency
 *
 *	inputs:
 *		frequency	frequency in Hz
 *
 *	output:
 *		1 or 0 depending on current state of the wave
 */
uint8_t get_square(uint8_t frequency)
{
	if(g_iWaveCursor % (BUFFER_SAMPLES / frequency) > (BUFFER_SAMPLES / 2 / frequency))
		return 1;
	else
		return 0;
}


/*
 *	Returns the value of a sawtooth wave at the current
 *	point in time, and at the requested frequency
 *
 *	inputs:
 *		frequency	frequency in Hz
 *
 *	output:
 *		float between [0-1] representing the current state
 *		of the wave
 */
float get_sawtooth(uint8_t frequency)
{
	return fmod((float) g_iWaveCursor / BUFFER_SAMPLES * frequency, 1);
}


/*
 *	Returns the value of an inverse sawtooth wave at the
 *	current point in time, and at the requested frequency
 *
 *	inputs:
 *		frequency	frequency in Hz
 *
 *	output:
 *		float between [0-1] representing the current state
 *		of the wave
 */
float get_inverse_sawtooth(uint8_t frequency)
{
	return 1 - get_sawtooth(frequency);
}


/*
 *	Returns the value of a triangle wave at the current
 *	point in time, and at the requested frequency
 *
 *	inputs:
 *		frequency	frequency in Hz
 *
 *	output:
 *		float between [0-1] representing the current state
 *		of the wave
 */
float get_triangle(uint8_t frequency)
{
	float calculation = fmod((float) g_iWaveCursor / BUFFER_SAMPLES * 2 * frequency, 2);

	if(calculation > 1)
		return 1 - (calculation - 1);

	return calculation;
}
