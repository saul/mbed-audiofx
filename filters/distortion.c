#include "config.h"
#include "dbg.h"
#include "samples.h"
#include "distortion.h"


uint32_t filter_bitcrusher_apply(uint32_t input, void *pUnknown)
{
	const FilterBitcrusherData_t *pData = (const FilterBitcrusherData_t *)pUnknown;

	return (input >> pData->bitLoss) << pData->bitLoss;
}


void filter_bitcrusher_debug(void *pUnknown)
{
	const FilterBitcrusherData_t *pData = (const FilterBitcrusherData_t *)pUnknown;
	dbg_printf("bitLoss=%u", pData->bitLoss);
}