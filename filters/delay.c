#include <stdint.h>

#include "usbcon.h"
#include "samples.h"
#include "delay.h"


uint32_t filter_delay_apply(uint32_t input, void *pPrivate)
{
	const FilterDelayData_t *pData = (const FilterDelayData_t *)pPrivate;
	return sample_get(-pData->nDelay) << 20;
}


void filter_delay_debug(void *pPrivate)
{
	const FilterDelayData_t *pData = (const FilterDelayData_t *)pPrivate;
	usbcon_writef("delay=%u", pData->nDelay);
}