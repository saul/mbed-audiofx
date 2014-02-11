#include <stdint.h>

#include "dbg.h"
#include "samples.h"
#include "flange.h"
#include "waves.h"

/*
	Flange works by assuming the current sample to be the one in the past.
	Using this assumption, it then combines this 'current' sample with ones
	a differing distance before and after it.
	For this reason, there may need to be a change to the overall filter chain,
	otherwise filters combined with a flange will be processing a 'newer' sample.
*/

uint32_t filter_flange_apply(uint32_t input, void *pPrivate)
{
	const FilterFlangeData_t *pData = (const FilterDelayData_t *)pPrivate;
	dbg_assert(pData->nDelay < BUFFER_SAMPLES / 2, "invalid flange parameter (nDelay)");
	uint16_t iCurrentFlange;
	uint32_t output = pData->originalMix * sample_get(-pData->nDelay);
	switch (pData->waveType)
	{
		case 0:
			if (get_square(pData->speed) == 1)
				return output + pData->flangedMix * sample_get(-pData->nDelay*2);
			else
				return output + pData->flangedMix * sample_get(0);
			break;
		case 1:
			return output +	pData->flangedMix * sample_get_interpolated(-get_sawtooth(pData->speed)*nDelay*2));
			break;
		case 2:
			return output + pData->flangedMix * sample_get_interpolated(-get_inverse_sawtooth(pData->speed)*nDelay*2);
			break;
		case 3:
			return output + pData->flangedMix * sample_get_interpolated(-get_triangle(pData->speed)*nDelay*2);
			break;
	}
}
