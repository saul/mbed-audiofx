/*
 *	HAPR Project 2014
 *	Group 6 - Tom Bryant (TB) & Saul Rennison (SR)
 *
 *	File created by:	SR
 *	File modified by:	TB & SR
 *	File debugged by:	TB & SR
 */


#ifndef _SAMPLES_H_
#define _SAMPLES_H_

#include <stdint.h>


extern volatile uint16_t g_iSampleCursor;
extern volatile uint16_t g_iWaveCursor;
extern volatile float g_flVibratoSampleCursor;


/*
 *	SamplePair data structure
 *	Because samples are only 12 bit, it is a waste of
 *	space to store them in 16 bits. By packing them
 *	together, two 12 bit values can fit into a 24 bit
 *	space, allowing more samples to be stored in memory.
 *
 *	Each individual sample is accessed using .a or .b
 */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-pedantic"
#pragma pack(push, 1)
typedef struct
{
	int16_t a : 12;
	int16_t b : 12;
} SamplePair_t;
#pragma pack(pop)
#pragma GCC diagnostic pop


typedef struct
{
	uint16_t nSamples;
	uint32_t average;
} SampleAverage_t;


int16_t sample_get(int16_t index);
void sample_set(int16_t index, int16_t value);
int16_t sample_get_interpolated(float index);
uint16_t sample_get_average(uint16_t nSamples);
void sample_clear_average(void);

#endif
