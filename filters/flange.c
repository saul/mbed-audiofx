/*
 *	HAPR Project 2014
 *	Group 6 - Tom Bryant (TB) & Saul Rennison (SR)
 *
 *	File created by:	TB
 *	File modified by:	TB
 *	File debugged by:	TB
 *
 *	flange.c
 *
 *	Defines functions to apply a flange effect to a sample.
*/


#include <stdint.h>

#include "dbg.h"
#include "config.h"
#include "samples.h"
#include "flange.h"
#include "waves.h"


/*
 *	Flange effect is created by combining the current 'input' with
 *	a sample a varying amount in the past.
 *
 *	inputs:
 *		input		signed 12 bit sample
 *		pUnknown	null pointer to FilterFlangeData_t data
 *
 *	output:
 *		signed 12 bit sample
 */
int16_t filter_flange_apply(int16_t input, void *pUnknown)
{
	const FilterFlangeData_t *pData = (const FilterFlangeData_t *)pUnknown;

	dbg_assert(pData->nDelay < BUFFER_SAMPLES / 2, "invalid flange parameter (nDelay)");

	int16_t output = (1 - pData->flangedMix) * input;

	float index;
	switch (pData->waveType)
	{
		case 0:
			if(get_square(pData->frequency))
				index = g_iSampleCursor;
			else
				index = g_iSampleCursor - pData->nDelay;
			break;
		case 1:
			index = g_iSampleCursor - get_sawtooth(pData->frequency) * pData->nDelay;
			break;
		case 2:
			index = g_iSampleCursor - get_inverse_sawtooth(pData->frequency) * pData->nDelay;
			break;
		case 3:
			index = g_iSampleCursor - get_triangle(pData->frequency) * pData->nDelay;
			break;
		default:
			index = g_iSampleCursor;
	}
	if(index <= 0)
	{
		index += g_iSampleCursor;
	}
	return output + pData->flangedMix * sample_get_interpolated(index);
}


// Print flange parameters to UI console
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdouble-promotion"
void filter_flange_debug(void *pUnknown)
{
	const FilterFlangeData_t *pData = (const FilterFlangeData_t *)pUnknown;
	dbg_printf("delay=%u, frequency=%u, waveType=%u, flangedMix=%f", pData->nDelay, pData->frequency, pData->waveType, pData->flangedMix);
}
#pragma GCC diagnostic pop


// Set initial creation flange parameter values
void filter_flange_create(void *pUnknown)
{
	FilterFlangeData_t *pData = (FilterFlangeData_t *)pUnknown;
	pData->nDelay = 10;
	pData->frequency = 1;
	pData->waveType = 0;
	pData->flangedMix = 0.5;
}
