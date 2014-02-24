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
#include "filters/fir.h"


#define PARAM_SEP "|"


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
	{
		"Delay",
		"Delay;f=H;o=0;t=range;min=0;max=9999;step=1;val=5000",
		filter_delay_apply, filter_delay_debug, NULL,
		sizeof(FilterDelayData_t), 0
	},

	{
		"Noise gate",
		"Sensitivity;f=H;o=0;t=range;min=1;max=9999;step=1;val=50" PARAM_SEP
		"Threshold;f=H;o=2;t=range;min=0;max=2048;step=1;val=200",
		filter_noisegate_apply, filter_noisegate_debug, NULL,
		sizeof(FilterNoiseGateData_t), 0
	},

	/*
	{
		"Vibrato",
		"", // TODO: fill in!
		filter_vibrato_apply, filter_vibrato_debug, NULL,
		sizeof(FilterVibratoData_t), 0
	},
	*/

	/*
	{
		"Tremolo",
		"", // TODO: fill in!
		filter_tremolo_apply, NULL, NULL, // TODO: add debug function
		sizeof(FilterTremoloData_t), 0
	},
	*/

	{
		"Band-pass",
		"Co-efficients;f=B;o=0;t=range;min=1;max=50;step=1;val=25" PARAM_SEP
		"Lower frequency;f=H;o=1;t=range;min=20;max=20000;step=1;val=20" PARAM_SEP
		"Upper frequency;f=H;o=3;t=range;min=20;max=20000;step=1;val=20000",
		filter_fir_apply, filter_bandpass_debug, filter_bandpass_mod,
		sizeof(FilterBandPassData_t), offsetof(FilterFIRBaseData_t, nCoefficients)
	},
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
