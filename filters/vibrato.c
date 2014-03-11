/*
 *	HAPR Project 2014
 *	Group 6 - Tom Bryant (TB) & Saul Rennison (SR)
 *
 *	File created by:	TB
 *	File modified by:	TB
 *	File debugged by:	TB
*/


#include "dbg.h"
#include "samples.h"
#include "vibrato.h"
#include "waves.h"
#include "config.h"


volatile bool g_bVibratoActive = false;

/*
 *	Returns the current cursor for the currently applied vibrato.
 *	Calculated by using the vibrato's FilterVibratoData_t data.
 */
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
				vib_cursor = g_iSampleCursor - (pData->nDelay);
		case 1:
			vib_cursor = g_iSampleCursor - (pData->nDelay * get_sawtooth(pData->frequency));
		case 2:
			vib_cursor = g_iSampleCursor - (pData->nDelay * get_inverse_sawtooth(pData->frequency));
		case 3:
			vib_cursor = g_iSampleCursor - (pData->nDelay * get_triangle(pData->frequency));
		default:
			vib_cursor = g_iSampleCursor;
	}

	vib_cursor += BUFFER_SAMPLES;

	while(vib_cursor > BUFFER_SAMPLES)
		vib_cursor -= BUFFER_SAMPLES;

	return vib_cursor;
}


/*
 *	Set the global vibrato boolean to true and
 *	calculate the current cursor value, before
 *	returning the sample in the delay line at
 *	the calculated cursor value.
 *
 *	inputs:
 *		input		signed 12 bit sample
 *		pUnknown	null pointer to FilterVibratoData_t data
 *
 *	output:
 *		signed 12 bit sample
 */
int16_t filter_vibrato_apply(int16_t input, void *pUnknown)
{
	g_bVibratoActive = true;
	g_flVibratoSampleCursor = vibrato_get_cursor(pUnknown);
	return sample_get(g_flVibratoSampleCursor);
}


// Print vibrato parameter values to UI console
void filter_vibrato_debug(void *pUnknown)
{
	const FilterVibratoData_t *pData = (const FilterVibratoData_t *)pUnknown;
	dbg_printf("delay=%u, frequency=%u, waveType=%u", pData->nDelay, pData->frequency, pData->waveType);
}


// Set initial creation vibrato parameter values
void filter_vibrato_create(void *pUnknown)
{
	FilterVibratoData_t *pData = (FilterVibratoData_t *)pUnknown;
	pData->nDelay = 10;
	pData->frequency = 1;
	pData->waveType = 0;
}
