#include "config.h"
#include "dbg.h"
#include "samples.h"
#include "distortion.h"


uint32_t filter_bitcrusher_apply(uint32_t input, void *pUnknown)
{
	const FilterBitcrusherData_t *pData = (const FilterBitcrusherData_t *)pUnknown;

	return (input >> (20 + pData->bitLoss)) << (20 + pData->bitLoss);
}


void filter_bitcrusher_debug(void *pUnknown)
{
	const FilterBitcrusherData_t *pData = (const FilterBitcrusherData_t *)pUnknown;
	dbg_printf("bitLoss=%u", pData->bitLoss);
}


void filter_bitcrusher_create(void *pUnknown)
{
	FilterBitcrusherData_t *pData = (FilterBitcrusherData_t *)pUnknown;
	pData->bitLoss = 1;
}
