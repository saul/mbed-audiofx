#include "dbg.h"
#include "samples.h"
#include "vibrato.h"
#include "waves.h"


float get_vibrato_pointer(void *pPrivate)
{
	const FilterVibratoData_t *pData = (const FilterVibratoData_t *)pPrivate;

	switch (pData->waveType)
	{
		case 0:
			if(get_square(pData->frequency))
				return g_iSampleCursor;
			else
				return g_iSampleCursor - (2 * pData->nDelay);
		case 1:
			return g_iSampleCursor - (pData->nDelay * 2 * get_sawtooth(pData->frequency));
		case 2:
			return g_iSampleCursor - (pData->nDelay * 2 * get_inverse_sawtooth(pData->frequency));
		case 3:
			return g_iSampleCursor - (pData->nDelay * 2 * get_triangle(pData->frequency));
		default:
			return input;
	}
}


void filter_noisegate_debug(void *pPrivate)
{
	const FilterNoiseGateData_t *pData = (const FilterNoiseGateData_t *)pPrivate;
	dbg_printf("sensitivity=%u, threshold=%u", pData->sensitivity, pData->threshold);
}
