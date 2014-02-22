#include <stdint.h>
#include <math.h>

#include "config.h"
#include "dbg.h"
#include "samples.h"


/*volatile*/ SamplePair_t g_pSampleBuffer[BUFFER_SAMPLES / 2];
volatile uint16_t g_iSampleCursor = 0;
volatile uint16_t g_iVibratoSampleCursor = 0;


SampleAverage_t sampleAverage;


uint32_t sample_get(int16_t index)
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


uint32_t sample_get_raw(int16_t index, bool bAffectedByVibrato)
{
	if(g_bVibratoActive && bAffectedByVibrato)
		return sample_get_interpolated(index);

	if(index & 1)
		return g_pSampleBuffer[(index-1)/2].b << 20;

	return g_pSampleBuffer[index/2].a << 20;
}


void sample_set(int16_t index, uint16_t value)
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


uint32_t sample_get_interpolated(float index)
{
	// return interpolated sample from past

	if(g_bVibratoActive)
	{
		if(index < 0)
			index = (g_iVibratoSampleCursor - g_iSampleCursor) + index;
		else
			index = (g_iSampleCursor - g_iVibratoSampleCursor) + index;
	}

	int16_t i;
	if(index < 0)
		i = (int)(index - 1);
	else
		i = (int) index;

	uint32_t si = sample_get_raw(i, false);
	uint32_t sj = sample_get_raw(i+1, false);

	return si + ((index - i) * (sj - si));
}


uint32_t sample_get_average(uint16_t nSamples)
{
	// Returns the average of the most recent 'nSamples' samples.
	// Result is the average distance from baseline to peak (result <= ADC_MAX_VALUE / 2).
	dbg_assert(nSamples <= BUFFER_SAMPLES, "requested average larger than buffer size");

	// Enable reuse of calculations by storing the first average calculated each sample
	// and returning it ifthe parameters match.
	if(sampleAverage.nSamples == nSamples)
		return sampleAverage.average;

	int32_t sum = 0;
	for(uint16_t i = 0; i < nSamples; ++i)
	{
		int32_t intermediate = sample_get(-i) >> 20;
		intermediate -= ((ADC_MAX_VALUE + 1) / 2);
		sum += intermediate * intermediate;
	}

	sum = ((int)sqrt(sum / nSamples)) << 20;
	if(sampleAverage.nSamples == 0)
	{
		sampleAverage.nSamples = nSamples;
		sampleAverage.average = sum;
	}
	return sum;
}


void sample_clear_average()
{
	sampleAverage.nSamples = 0;
}
