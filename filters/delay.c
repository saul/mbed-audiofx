#include <stdint.h>

#include "dbg.h"
#include "samples.h"
#include "delay.h"


uint32_t filter_delay_apply(uint32_t input, void *pUnknown)
{
	const FilterDelayData_t *pData = (const FilterDelayData_t *)pUnknown;

	if(pData->nDelay == 0)
		return sample_get(g_iSampleCursor);

	return sample_get(-pData->nDelay);
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
}
