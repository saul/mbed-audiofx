#include <stdint.h>

#include "dbg.h"
#include "samples.h"
#include "delay.h"


int32_t filter_delay_apply(int32_t input, void *pUnknown)
{
	const FilterDelayData_t *pData = (const FilterDelayData_t *)pUnknown;

	if(pData->nDelay == 0)
		return (sample_get(g_iSampleCursor) * pData->flDelayMixPerc) + (input * (1-pData->flDelayMixPerc));

	return (sample_get(-pData->nDelay) * pData->flDelayMixPerc) + (input * (1-pData->flDelayMixPerc));
}


void filter_delay_debug(void *pUnknown)
{
	const FilterDelayData_t *pData = (const FilterDelayData_t *)pUnknown;
	dbg_printf("delay=%u", pData->nDelay);
}


void filter_delay_create(void *pUnknown)
{
	FilterDelayData_t *pData = (FilterDelayData_t *)pUnknown;
	pData->nDelay = 5000;
	pData->flDelayMixPerc = 0.5;
}


int32_t filter_delay_feedback_apply(int32_t input, void *pUnknown)
{
	int32_t result = filter_delay_apply(input, pUnknown);
	sample_set(g_iSampleCursor, result);

	return result;
}
