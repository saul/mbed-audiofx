/*
 * filters.c - Audio effect filters
 */

#include <stdlib.h>
#include <stdint.h>

#include "dbg.h"
#include "filters.h"
#include "filters/delay.h"
#include "filters/dynamic.h"
#include "filters/vibrato.h"
#include "filters/tremolo.h"


/*
 * g_pFilters
 *
 * Global list of filter types.
 *
 * Parameters:
 * - f: struct.pack format character
 *      (see http://docs.python.org/2/library/struct.html#format-characters)
 * - o: offset into private data
 * - t: widget type (range or choice)
 */
Filter_t g_pFilters[] = {
	{"Delay", "Delay;f=H;o=0;t=range;min=0;max=9999;step=1;val=5000", filter_delay_apply, filter_delay_debug, sizeof(FilterDelayData_t)},
	{"Noise gate", "Sensitivity;f=H;o=0;t=range;min=1;max=9999;step=1;val=50|Threshold;f=H;o=2;t=range;min=0;max=2048;step=1;val=200", filter_noisegate_apply, filter_noisegate_debug, sizeof(FilterNoiseGateData_t)},
	{"Vibrato", "", filter_vibrato_apply, filter_vibrato_debug, sizeof(FilterVibratoData_t)},
	{"Tremolo", "", filter_tremolo_apply, filter_tremolo_debug, sizeof(FilterTremoloData_t)},
};

const size_t NUM_FILTERS = sizeof(g_pFilters)/sizeof(g_pFilters[0]);


/*
 * filter_debug
 *
 * Print a list of available filter types.
 */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-pedantic" // ignore "function pointer -> void pointer" error
void filter_debug(void)
{
	dbg_printf(" === filter_debug (%u filters) ===\r\n\r\n", NUM_FILTERS);

	for(uint8_t i = 0; i < NUM_FILTERS; ++i)
	{
		const Filter_t *pFilter = &g_pFilters[i];
		dbg_printf("#%02u: %s, apply=%p, debug=%p, privateDataSize=%u\r\n", i, pFilter->pszName, (void *)pFilter->pfnApply, (void *)pFilter->pfnDebug, pFilter->nPrivateDataSize);
	}

	dbg_printn("\r\n", -1);
}
#pragma GCC diagnostic pop
