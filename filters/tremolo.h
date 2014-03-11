/*
 *	HAPR Project 2014
 *	Group 6 - Tom Bryant (TB) & Saul Rennison (SR)
 *
 *	File created by:	TB
 *	File modified by:	TB
 *	File debugged by:	TB
 *
 *	tremolo.c
 *
 *	Defines functions to apply a tremolo effect to a sample.
*/


#ifndef _FILTER_TREMOLO_H_
#define _FILTER_TREMOLO_H_


// Tremolo paramter data structure
typedef struct
{
	uint8_t frequency;	///< Frequency of the LFO (Hz) (Divisor of SAMPLE_RATE/4)
	uint8_t waveType;	///< 0 = Square, 1 = Sawtooth, 2 = Inverse Sawtooth, 3 = Triangle
	float depth;		///< Minimum amplitude scalar [0-1]
} FilterTremoloData_t;

int16_t filter_tremolo_apply(int16_t input, void *pUnknown);
void filter_tremolo_debug(void *pUnknown);
void filter_tremolo_create(void *pUnknown);

#endif
