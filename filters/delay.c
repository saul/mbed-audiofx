/*
 *	HAPR Project 2014
 *	Group 6 - Tom Bryant (TB) & Saul Rennison (SR)
 *
 *	File created by:	SR
 *	File modified by:	TB & SR
 *	File debugged by:	TB & SR
 *
 *	delay.c
 *
 *	Defines functions to apply a delay to a sample.
 */


#include <stdint.h>

#include "dbg.h"
#include "samples.h"
#include "delay.h"


/*
 *	Delay filter adds together the current 'input' with a sample
 *	'pData->nDelay' in the past, at a ratio of
 *	1 - 'pData->flDelayMixPerc' : 'pData->flDelayMixPerc'.
 *
 *	inputs:
 *		input		signed 12 bit audio sample
 *		pUnknown	null pointer to a FilterDelayData_t data structure
 *
 *	output:
 *		signed 12 bit audio sample
 */
int16_t filter_delay_apply(int16_t input, void *pUnknown)
{
	const FilterDelayData_t *pData = (const FilterDelayData_t *)pUnknown;

	if(pData->nDelay == 0)
		return (sample_get(g_iSampleCursor) * pData->flDelayMixPerc) + (input * (1-pData->flDelayMixPerc));

	return (sample_get(-pData->nDelay) * pData->flDelayMixPerc) + (input * (1-pData->flDelayMixPerc));
}


// Prints delay filter parameter information to UI console
void filter_delay_debug(void *pUnknown)
{
	const FilterDelayData_t *pData = (const FilterDelayData_t *)pUnknown;
	dbg_printf("delay=%u", pData->nDelay);
}


// Set delay filter initial creation values
void filter_delay_create(void *pUnknown)
{
	FilterDelayData_t *pData = (FilterDelayData_t *)pUnknown;
	pData->nDelay = 5000;
	pData->flDelayMixPerc = 0.5;
}


/*
 *	Delay with feedback applies writeback facility.
 *	The current sample will be overwritten with the two mixed samples
*/
int16_t filter_delay_feedback_apply(int16_t input, void *pUnknown)
{
	int16_t result = filter_delay_apply(input, pUnknown);
	sample_set(g_iSampleCursor, result);
	return result;
}
