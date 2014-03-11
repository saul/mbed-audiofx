#include "dbg.h"
#include "samples.h"
#include "vibrato.h"
#include "waves.h"
#include "config.h"


volatile bool g_bVibratoActive = false;


float vibrato_get_cursor(void *pUnknown)
{
	const FilterVibratoData_t *pData = (const FilterVibratoData_t *)pUnknown;

	float vib_cursor;

	switch (pData->waveType)
	{
		case 0:
			if(get_square(pData->frequency))
				vib_cursor = g_iSampleCursor;
			else
				vib_cursor = g_iSampleCursor - (2.0f * pData->nDelay);
		case 1:
			vib_cursor = g_iSampleCursor - (pData->nDelay * 2.0f * get_sawtooth(pData->frequency));
		case 2:
			vib_cursor = g_iSampleCursor - (pData->nDelay * 2.0f * get_inverse_sawtooth(pData->frequency));
		case 3:
			vib_cursor = g_iSampleCursor - (pData->nDelay * 2.0f * get_triangle(pData->frequency));
		default:
			vib_cursor = g_iSampleCursor;
	}

	vib_cursor += BUFFER_SAMPLES;

	while(vib_cursor > BUFFER_SAMPLES)
		vib_cursor -= BUFFER_SAMPLES;

	return vib_cursor;
}


int16_t filter_vibrato_apply(int16_t input, void *pUnknown)
{
	g_bVibratoActive = true;
	g_flVibratoSampleCursor = vibrato_get_cursor(pUnknown);
	return sample_get(g_iSampleCursor);
}


void filter_vibrato_debug(void *pUnknown)
{
	const FilterVibratoData_t *pData = (const FilterVibratoData_t *)pUnknown;
	dbg_printf("delay=%u, frequency=%u, waveType=%u", pData->nDelay, pData->frequency, pData->waveType);
}


void filter_vibrato_create(void *pUnknown)
{
	FilterVibratoData_t *pData = (FilterVibratoData_t *)pUnknown;
	pData->nDelay = 10;
	pData->frequency = 1;
	pData->waveType = 0;
}
