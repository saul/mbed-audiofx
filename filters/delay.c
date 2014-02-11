#include <stdint.h>

#include "dbg.h"
#include "samples.h"
#include "delay.h"


uint32_t filter_delay_apply(uint32_t input, void *pPrivate)
{
	const FilterDelayData_t *pData = (const FilterDelayData_t *)pPrivate;
	return sample_get(-pData->nDelay);
}


void filter_delay_debug(void *pPrivate)
{
	const FilterDelayData_t *pData = (const FilterDelayData_t *)pPrivate;
	dbg_printf("delay=%u", pData->nDelay);
}
