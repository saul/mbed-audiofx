/*
 *	HAPR Project 2014
 *	Group 6 - Tom Bryant (TB) & Saul Rennison (SR)
 *
 *	File created by:	SR
 *	File modified by:	TB & SR
 *	File debugged by:	TB & SR
 */


#include <stdint.h>
#include <stdbool.h>
#include <math.h>

#include "config.h"
#include "dbg.h"
#include "samples.h"
#include "filters/vibrato.h"


SamplePair_t g_pSampleBuffer[BUFFER_SAMPLES / 2];
volatile uint16_t g_iSampleCursor = 0;
volatile uint16_t g_iWaveCursor = 0;
volatile float g_flVibratoSampleCursor = 0;
static SampleAverage_t s_SampleAverage;


/*
 *	Returns the sample in the sample buffer at the requested index
 *	If 'bAffectedByVibrato' is true, 'g_bVibratoActive' is checked
 *	and if true, 'sample_get_interpolated' is called instead.
 *
 *	inputs:
 *		index				the index of the sample to be obtained
 *							[0-'MAX_BUFFER_SIZE']
 *		bAffectedByVibrato	bool, whether to take vibrato into account
 *
 *	output:
 *		signed 12 bit sample
 */
static int32_t sample_get_raw(int16_t index, bool bAffectedByVibrato)
{
	// let sample_get_interpolated deal with vibrato being active
	if(g_bVibratoActive && bAffectedByVibrato)
		return sample_get_interpolated(index);

	// Unpack a or b from the sample buffer, depending on index
	if(index & 1)
		return g_pSampleBuffer[(index-1)/2].b;

	return g_pSampleBuffer[index/2].a;
}


/*
 *	Returns a sample from the sample buffer.
 *	If 'index' is positive, return the sample at
 *	that position in the sample buffer array.
 *	If 'index' is negative, return the sample which
 *	is that many in the "past" from the current position.
 *
 *	inputs:
 *		index	the index of the sample to be obtained
 * 				[-9999 - 9999]
 *
 *	output:
 *		signed 12 bit sample
 */
int16_t sample_get(int16_t index)
{
	// return sample from past
	if(index < 0)
	{
		dbg_assert(index > -BUFFER_SAMPLES, "invalid sample index");
		// recall function with the corresponding positive integer
		return sample_get((BUFFER_SAMPLES + g_iSampleCursor + index) % BUFFER_SAMPLES);
	}

	dbg_assert(index < BUFFER_SAMPLES, "invalid sample index");

	// use sample_get_raw to take vibrato into account
	return sample_get_raw(index, true);
}


/*
 *	Sets a sample in the sample buffer.
 *	If 'index' is positive, set the sample at
 *	that position in the sample buffer array.
 *	If 'index' is negative, set the sample which
 *	is that many in the "past" from the current position.
 *
 *	inputs:
 *		index	the index of the sample to be obtained
 *				[-9999 - 9999]
 *		value	signed 12 bit sample to be placed into
 *				the buffer
 *
 */
void sample_set(int16_t index, int16_t value)
{
	// set sample from past
	if(index < 0)
	{
		dbg_assert(index > -BUFFER_SAMPLES, "invalid sample index");
		sample_set((BUFFER_SAMPLES + g_iSampleCursor + index) % BUFFER_SAMPLES, value);
		return;
	}

	dbg_assert(index < BUFFER_SAMPLES, "invalid sample index");

	if(index & 1)
		g_pSampleBuffer[(index-1)/2].b = value;
	else
		g_pSampleBuffer[index/2].a = value;
}


/*
 *	Returns an interpolated sample from the sample buffer.
 *	If 'index' is positive, return the sample at
 *	that position in the sample buffer array.
 *	If 'index' is negative, return the sample which
 *	is that many in the "past" from the current position.
 *	The interpolated sample is calculated by taking the
 *	integer samples before and after the float indexed
 *	sample, and linearly interpolating between them.
 *
 *	inputs:
 *		index	the index of the sample to be obtained
 * 				(float)[-10000 - 10000]
 *
 *	output:
 *		signed 12 bit sample
 */
int16_t sample_get_interpolated(float index)
{
	// If vibrato active, use vibrato pointer as base
	if(g_bVibratoActive)
	{
		if(index < 0)
			index = (g_flVibratoSampleCursor + index);
		else
			index = index + (g_flVibratoSampleCursor - g_iSampleCursor);
			// index = (g_iSampleCursor - g_flVibratoSampleCursor) + index;
		index += BUFFER_SAMPLES;
		while(index > BUFFER_SAMPLES)
			index -= BUFFER_SAMPLES;
	}

	// Get previous and next sample
	int16_t i;
	if(index < 0)
		i = (int)(index - 1);
	else
		i = (int) index;

	int16_t si = sample_get_raw(i, false);
	int16_t sj = sample_get_raw(i+1, false);

	// Interpolate
	return (int16_t) (si + ((index - i) * (sj - si)));
}


/*
 *	Returns the average peak amplitude of the previous 'nSamples'
 *	samples.
 *	As averaging lots of samples takes time, the first average
 *	collected is stored to allow multiple effects to use the
 *	same calculation, if their 'nSamples' is the same.
 *
 *	inputs:
 *		nSamples	number of samples to take an average over
 *
 *	output:
 *		unsigned value [0-2047]
 */
uint16_t sample_get_average(uint16_t nSamples)
{
	// Returns the average of the most recent 'nSamples' samples.
	// Result is the average distance from baseline to peak (result <= ADC_MAX_VALUE / 2).
	dbg_assert(nSamples <= BUFFER_SAMPLES, "requested average larger than buffer size");

	// Enable reuse of calculations by storing the first average calculated each sample
	// and returning it if the parameters match.
	if(s_SampleAverage.nSamples == nSamples)
		return s_SampleAverage.average;

	int32_t sum = 0;
	for(uint16_t i = 0; i < nSamples; ++i)
	{
		int32_t intermediate = (i == 0) ? sample_get(g_iSampleCursor) : sample_get(-i);
		sum += intermediate * intermediate;
	}

	// Set the sample average
	sum = ((int)sqrt(sum / nSamples));
	if(s_SampleAverage.nSamples == 0)
	{
		s_SampleAverage.nSamples = nSamples;
		s_SampleAverage.average = sum;
	}
	return sum;
}


// Clear the sample average
void sample_clear_average()
{
	s_SampleAverage.nSamples = 0;
}
