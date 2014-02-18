#include "dbg.h"
#include "samples.h"
#include "dynamic.h"

uint32_t filter_noisegate_apply(uint32_t input, void *pPrivate)
{
	const FilterNoisegateData_t *pData = (const FilterNoisegateData_t *)pPrivate;

	uint32_t average = sample_get_average(pData->sensitivity);
	if (average < threshold)
		return 0;
	else
		return input;
}