#include "dbg.h"
#include "samples.h"
#include "dynamic.h"
#include "config.h"


/*
Noisegate filter takes an average of the amplitude over the last
'pData->sensitivity' samples. If the average is lower than
'pData->threshold', silence is returned. Otherwise, the input
is returned the same as the input.
*/
uint32_t filter_noisegate_apply(uint32_t input, void *pUnknown)
{
	const FilterNoiseGateData_t *pData = (const FilterNoiseGateData_t *)pUnknown;

	uint16_t average = sample_get_average(pData->sensitivity) >> 20;
	if(average < pData->threshold)
		return ADC_MID_POINT << 20;
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
	pData->sensitivity = 25;
	pData->threshold = 200;
}


uint32_t filter_compressor_apply(uint32_t input, void *pUnknown)
{
	const FilterCompressorData_t *pData = (const FilterCompressorData_t *)pUnknown;
	input = input >> 20;

	uint16_t average = sample_get_average(pData->sensitivity) >> 20;
	if(average < pData->threshold)
		return input;

	int32_t signed_input = input - ADC_MID_POINT;
	signed_input = signed_input * pData->scalar;
	signed_input += ADC_MID_POINT;
	if(signed_input < 0)
		return 0;
	if(signed_input > ADC_MAX_VALUE)
		return ADC_MAX_VALUE << 20;
	return (uint32_t)(signed_input << 20);
}


void filter_compressor_debug(void *pUnknown)
{
	const FilterCompressorData_t *pData = (const FilterCompressorData_t *)pUnknown;
	dbg_printf("sensitivity=%u, threshold=%u, scalar=%f", pData->sensitivity, pData->threshold, pData->scalar);
}


void filter_compressor_create(void *pUnknown)
{
	FilterCompressorData_t *pData = (FilterCompressorData_t *)pUnknown;
	pData->sensitivity = 25;
	pData->threshold = 200;
	pData->scalar = 0.8;
}


uint32_t filter_expander_apply(uint32_t input, void *pUnknown)
{
	const FilterCompressorData_t *pData = (const FilterCompressorData_t *)pUnknown;
	input = input >> 20;

	uint16_t average = sample_get_average(pData->sensitivity) >> 20;
	if(average > pData->threshold)
		return input;

	int32_t signed_input = input - ADC_MID_POINT;
	signed_input = signed_input * pData->scalar;
	signed_input += ADC_MID_POINT;
	if(signed_input < 0)
		return 0;
	if(signed_input > ADC_MAX_VALUE)
		return ADC_MAX_VALUE << 20;
	return (uint32_t)(signed_input << 20);
}


void filter_expander_create(void *pUnknown)
{
	FilterCompressorData_t *pData = (FilterCompressorData_t *)pUnknown;
	pData->sensitivity = 25;
	pData->threshold = 200;
	pData->scalar = 1.5;
}
