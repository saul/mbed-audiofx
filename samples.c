#include <stdint.h>
#include <stdbool.h>
#include <math.h>

#include "config.h"
#include "dbg.h"
#include "samples.h"
#include "filters/vibrato.h"


/*volatile*/ SamplePair_t g_pSampleBuffer[BUFFER_SAMPLES / 2];
volatile uint16_t g_iSampleCursor = 0;
volatile uint16_t g_iWaveCursor = 0;
volatile float g_flVibratoSampleCursor = 0;
static SampleAverage_t s_SampleAverage;


static int32_t sample_get_raw(int16_t index, bool bAffectedByVibrato)
{
	if(g_bVibratoActive && bAffectedByVibrato)
		return sample_get_interpolated(index);

	if(index & 1)
		return g_pSampleBuffer[(index-1)/2].b;

	return g_pSampleBuffer[index/2].a;
}


int16_t sample_get(int16_t index)
{
	// return sample from past
	if(index < 0)
	{
		dbg_assert(index > -BUFFER_SAMPLES, "invalid sample index");
		return sample_get((BUFFER_SAMPLES + g_iSampleCursor + index) % BUFFER_SAMPLES);
	}

	dbg_assert(index < BUFFER_SAMPLES, "invalid sample index");

	return sample_get_raw(index, true);
}


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


int16_t sample_get_interpolated(float index)
{
	// return interpolated sample from past

	if(g_bVibratoActive)
	{
		index = (g_iSampleCursor - g_flVibratoSampleCursor) + index + BUFFER_SAMPLES;
		if(index > BUFFER_SAMPLES)
			index = index - BUFFER_SAMPLES;
	}

	int16_t i;
	if(index < 0)
		i = (int)(index - 1);
	else
		i = (int) index;

	int16_t si = sample_get_raw(i, false);
	int16_t sj = sample_get_raw(i+1, false);

	return si + ((index - i) * (sj - si));
}


uint32_t sample_get_average(uint16_t nSamples)
{
	// Returns the average of the most recent 'nSamples' samples.
	// Result is the average distance from baseline to peak (result <= ADC_MAX_VALUE / 2).
	dbg_assert(nSamples <= BUFFER_SAMPLES, "requested average larger than buffer size");

	// Enable reuse of calculations by storing the first average calculated each sample
	// and returning it ifthe parameters match.
	if(s_SampleAverage.nSamples == nSamples)
		return s_SampleAverage.average;

	int32_t sum = 0;
	for(uint16_t i = 0; i < nSamples; ++i)
	{
		int32_t intermediate = sample_get(-i);
		sum += intermediate * intermediate;
	}

	sum = ((int)sqrt(sum / nSamples));
	if(s_SampleAverage.nSamples == 0)
	{
		s_SampleAverage.nSamples = nSamples;
		s_SampleAverage.average = sum;
	}
	return sum;
}


void sample_clear_average()
{
	s_SampleAverage.nSamples = 0;
}
