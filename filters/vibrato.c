#include "dbg.h"
#include "samples.h"
#include "vibrato.h"
#include "waves.h"


volatile bool g_bVibratoActive = false;


float vibrato_get_cursor(void *pUnknown)
{
	const FilterVibratoData_t *pData = (const FilterVibratoData_t *)pUnknown;

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


uint32_t filter_vibrato_apply(uint32_t input, void *pUnknown)
{
	g_bVibratoActive = true;
	return input;
}


void filter_vibrato_debug(void *pUnknown)
{
	const FilterVibratoData_t *pData = (const FilterVibratoData_t *)pUnknown;
	dbg_printf("nDelay=%u, frequency=%u, waveType=%u", pData->nDelay, pData->frequency, pData->waveType);
}


void filter_vibrato_create(void *pUnknown)
{
	FilterVibratoData_t *pData = (FilterVibratoData_t *)pUnknown;
	pData->nDelay = 10;
	pData->frequency = 1;
	pData->waveType = 0;
}
