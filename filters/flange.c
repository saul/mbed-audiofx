#include <stdint.h>

#include "dbg.h"
#include "config.h"
#include "samples.h"
#include "flange.h"
#include "waves.h"

// Flange works by assuming the current sample to be the one in the past.
// Using this assumption, it then combines this 'current' sample with ones
// a differing distance before and after it.
// For this reason, there may need to be a change to the overall filter chain,
// otherwise filters combined with a flange will be processing a 'newer' sample

int16_t filter_flange_apply(int16_t input, void *pUnknown)
{
	const FilterFlangeData_t *pData = (const FilterFlangeData_t *)pUnknown;

	// TODO: move to filter_flange_validate. We shouldn't have assertions or
	//       parameter checks in the filter apply code
	dbg_assert(pData->nDelay < BUFFER_SAMPLES / 2, "invalid flange parameter (nDelay)");

	int16_t output = (1 - pData->flangedMix) * sample_get(-pData->nDelay);

	// TODO: validate waveType value
	float index;
	switch (pData->waveType)
	{
		case 0:
			if(get_square(pData->frequency))
				return output + pData->flangedMix * sample_get(-pData->nDelay*2);
			else
				return output + pData->flangedMix * sample_get(g_iSampleCursor);
		case 1:
			index = -get_sawtooth(pData->frequency)*pData->nDelay*2;
			if(index == 0)
			{
				index = g_iSampleCursor;
			}
			return output + pData->flangedMix * sample_get_interpolated(index);
		case 2:
			index = -get_inverse_sawtooth(pData->frequency)*pData->nDelay*2;
			if(index == 0)
			{
				index = g_iSampleCursor;
			}
			return output + pData->flangedMix * sample_get_interpolated(index);
		case 3:
			index = -get_triangle(pData->frequency)*pData->nDelay*2;
			if(index == 0)
			{
				index = g_iSampleCursor;
			}
			return output + pData->flangedMix * sample_get_interpolated(index);
		default:
			return 0;
	}
}


#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdouble-promotion"
void filter_flange_debug(void *pUnknown)
{
	const FilterFlangeData_t *pData = (const FilterFlangeData_t *)pUnknown;
	dbg_printf("nDelay=%u, frequency=%u, waveType=%u, flangedMix=%f", pData->nDelay, pData->frequency, pData->waveType, pData->flangedMix);
}
#pragma GCC diagnostic pop


void filter_flange_create(void *pUnknown)
{
	FilterFlangeData_t *pData = (FilterFlangeData_t *)pUnknown;
	pData->nDelay = 10;
	pData->frequency = 1;
	pData->waveType = 0;
	pData->flangedMix = 0.5;
}
