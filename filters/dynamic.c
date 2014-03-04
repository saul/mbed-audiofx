#include "dbg.h"
#include "samples.h"
#include "dynamic.h"


uint32_t filter_noisegate_apply(uint32_t input, void *pUnknown)
{
	const FilterNoiseGateData_t *pData = (const FilterNoiseGateData_t *)pUnknown;

	uint16_t average = sample_get_average(pData->sensitivity) >> 20;
	if(average < pData->threshold)
		return 0;
	else
		return input;
}


void filter_noisegate_debug(void *pUnknown)
{
	const FilterNoiseGateData_t *pData = (const FilterNoiseGateData_t *)pUnknown;
	dbg_printf("sensitivity=%u, threshold=%u", pData->sensitivity, pData->threshold);
}


void filter_noisegate_create(void *pUnknown)
{
	FilterNoiseGateData_t *pData = (FilterNoiseGateData_t *)pUnknown;
	pData->sensitivity = 50;
	pData->threshold = 200;
}


uint32_t filter_compressor_apply(uint32_t input, void *pUnknown)
{
	const FilterCompressorData_t *pData = (const FilterCompressorData_t *)pUnknown;

	uint16_t average = sample_get_average(pData->sensitivity) >> 20;
	if(average < pData->threshold)
		return input;
	else
		return input * pData->scalar;
}


void filter_compressor_debug(void *pUnknown)
{
	const FilterCompressorData_t *pData = (const FilterCompressorData_t *)pUnknown;
	dbg_printf("sensitivity=%u, threshold=%u, scalar=%f", pData->sensitivity, pData->threshold, pData->scalar);
}


void filter_compressor_create(void *pUnknown)
{
	FilterCompressorData_t *pData = (FilterCompressorData_t *)pUnknown;
	pData->sensitivity = 50;
	pData->threshold = 200;
	pData->scalar = 0.8;
}


uint32_t filter_expander_apply(uint32_t input, void *pUnknown)
{
	const FilterCompressorData_t *pData = (const FilterCompressorData_t *)pUnknown;

	uint16_t average = sample_get_average(pData->sensitivity) >> 20;
	if(average > pData->threshold)
		return input;
	else
		return input * pData->scalar;
}


void filter_expander_create(void *pUnknown)
{
	FilterCompressorData_t *pData = (FilterCompressorData_t *)pUnknown;
	pData->sensitivity = 50;
	pData->threshold = 200;
	pData->scalar = 1.5;
}
