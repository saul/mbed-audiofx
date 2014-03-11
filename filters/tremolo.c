#include <stdint.h>

#include "dbg.h"
#include "config.h"
#include "samples.h"
#include "waves.h"
#include "tremolo.h"


int16_t filter_tremolo_apply(int16_t input, void *pUnknown)
{
	const FilterTremoloData_t *pData = (const FilterTremoloData_t *)pUnknown;

	switch (pData->waveType)
	{
		case 0:
			if(get_square(pData->frequency))
				return input;
			else
				return input * (1 - pData->depth);
		case 1:
			return input * ((1 - pData->depth) + (get_sawtooth(pData->frequency) * pData->depth));
		case 2:
			return input * ((1 - pData->depth) + (get_inverse_sawtooth(pData->frequency) * pData->depth));
		case 3:
			return input * ((1 - pData->depth) + (get_triangle(pData->frequency) * pData->depth));
		default:
			return input;
	}
}


#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdouble-promotion"
void filter_tremolo_debug(void *pUnknown)
{
	const FilterTremoloData_t *pData = (const FilterTremoloData_t *)pUnknown;
	dbg_printf("frequency=%u, waveType=%u, depth=%f", pData->frequency, pData->waveType, pData->depth);
}
#pragma GCC diagnostic pop


void filter_tremolo_create(void *pUnknown)
{
	FilterTremoloData_t *pData = (FilterTremoloData_t *)pUnknown;
	pData->frequency = 1;
	pData->waveType = 0;
	pData->depth = 0.5;
}
