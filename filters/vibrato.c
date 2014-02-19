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
			return g_iSampleCursor;
	}
}


uint32_t filter_vibrato_apply(uint32_t input, void *pPrivate)
{
	g_bVibratoActive = true;
	return input;
}


void filter_vibrato_debug(void *pPrivate)
{
	const FilterVibratoData_t *pData = (const FilterVibratoData_t *)pPrivate;
	dbg_printf("nDelay=%u, frequency=%u, waveType=%u", pData->nDelay, pData->frequency, pData->waveType);
}
