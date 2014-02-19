#include <stdint.h>

#include "dbg.h"
#include "config.h"
#include "samples.h"
#include "waves.h"
#include "tremolo.h"


uint32_t filter_tremolo_apply(uint32_t input, void *pPrivate)
{
	const FilterTremoloData_t *pData = (const FilterTremoloData_t *)pPrivate;

	switch (pData->waveType)
	{
		case 0:
			if(get_square(pData->speed))
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
