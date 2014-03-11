/*
 *	HAPR Project 2014
 *	Group 6 - Tom Bryant (TB) & Saul Rennison (SR)
 *
 *	File created by:	TB
 *	File modified by:	TB
 *	File debugged by:	TB
 *
 *	flange.c
 *
 *	Defines functions to apply a flange effect to a sample.
*/


#ifndef _FILTER_FLANGE_H_
#define _FILTER_FLANGE_H_


// Structure for Flange data
typedef struct
{
	uint16_t nDelay;	///< The maximum sample in the past to go to [0-9999]
	uint8_t frequency;	///< The frequency of the LFO (Hz)
	uint8_t waveType;	///< 0 = Square, 1 = Sawtooth, 2 = Inverse Sawtooth, 3 = Triangle
	float flangedMix;	///< The mix amount of the flanged part [0-1]
} FilterFlangeData_t;


int16_t filter_flange_apply(int16_t input, void *pUnknown);
void filter_flange_debug(void *pUnknown);
void filter_flange_create(void *pUnknown);

#endif
