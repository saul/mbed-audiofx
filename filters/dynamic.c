#include "dbg.h"
#include "samples.h"
#include "dynamic.h"


uint32_t filter_noisegate_apply(uint32_t input, void *pPrivate)
{
	const FilterNoiseGateData_t *pData = (const FilterNoiseGateData_t *)pPrivate;

	uint16_t average = sample_get_average(pData->sensitivity) >> 20;
	if (average < pData->threshold)
		return 0;
	else
		return input;
}


void filter_noisegate_debug(void *pPrivate)
{
	const FilterNoiseGateData_t *pData = (const FilterNoiseGateData_t *)pPrivate;
	dbg_printf("sensitivity=%u, threshold=%u", pData->sensitivity, pData->threshold);
}
