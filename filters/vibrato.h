/*
 *	HAPR Project 2014
 *	Group 6 - Tom Bryant (TB) & Saul Rennison (SR)
 *
 *	File created by:	TB
 *	File modified by:	TB
 *	File debugged by:	TB
*/


#ifndef _FILTER_VIBRATO_H_
#define _FILTER_VIBRATO_H_

#include <stdbool.h>


// Structure to hold vibrato parameter values
typedef struct
{
	uint16_t nDelay;	///< The maximum sample backward to go [0-9999]
	uint8_t frequency;	///< The frequency of the LFO used
	uint8_t waveType;	///< 0 = Square, 1 = Sawtooth, 2 = Inverse Sawtooth, 3 = Triangle
} FilterVibratoData_t;


extern volatile bool g_bVibratoActive;


float vibrato_get_cursor(void *pUnknown);
int16_t filter_vibrato_apply(int16_t input, void *pUnknown);
void filter_vibrato_debug(void *pUnknown);
void filter_vibrato_create(void *pUnknown);

#endif
