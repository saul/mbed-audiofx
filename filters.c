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
#include "filters/oscillating_fir.h"
#include "filters/distortion.h"

/*
 * g_pFilters
 *
 * Global list of filter types.
 *
 * Parameters:
 * - f: struct.pack format character
 *      (see http://docs.python.org/2/library/struct.html#format-characters)
 * - o: offset into filter data struct
 * - t: widget type (range or choice)
 */
#define WAVE_TYPE_KV ";f=B;t=choice;c=Square;c=Sawtooth;c=Inverse Sawtooth;c=Triangle"

Filter_t g_pFilters[] = {
	{
		"Delay",
		"Delay;f=H;o=0;t=range;min=0;max=9999;step=1;val=5000" PARAM_SEP
		"Mix level;f=f;o=2;t=range;min=0;max=1;step=0.05;val=0.5",
		filter_delay_apply, filter_delay_debug, filter_delay_create, NULL,
		sizeof(FilterDelayData_t), 0
	},

	{
		"Reverb",
		"Delay;f=H;o=0;t=range;min=0;max=9999;step=1;val=5000" PARAM_SEP
		"Mix level;f=f;o=2;t=range;min=0;max=1;step=0.05;val=0.5",
		filter_delay_feedback_apply, filter_delay_debug, filter_delay_create, NULL, // Using delay as they share data structure
		sizeof(FilterDelayData_t), 0
	},

	{
		"Noise Gate",
		"Sensitivity;f=H;o=0;t=range;min=1;max=100;step=1;val=25" PARAM_SEP
		"Threshold;f=H;o=2;t=range;min=0;max=1433;step=1;val=200",
		filter_noisegate_apply, filter_noisegate_debug, filter_noisegate_create, NULL,
		sizeof(FilterNoiseGateData_t), 0
	},

	{
		"Compressor",
		"Sensitivity;f=H;o=0;t=range;min=1;max=100;step=1;val=25" PARAM_SEP
		"Threshold;f=H;o=2;t=range;min=0;max=1433;step=1;val=200" PARAM_SEP
		"Scalar;f=f;o=4;t=range;min=0;max=1;step=0.05;val=0.8",
		filter_compressor_apply, filter_compressor_debug, filter_compressor_create, NULL,
		sizeof(FilterCompressorData_t), 0
	},

	{
		"Expander",
		"Sensitivity;f=H;o=0;t=range;min=1;max=100;step=1;val=25" PARAM_SEP
		"Threshold;f=H;o=2;t=range;min=0;max=1433;step=1;val=200" PARAM_SEP
		"Scalar;f=f;o=4;t=range;min=1;max=2;step=0.05;val=1.5",
		filter_expander_apply, filter_compressor_debug, filter_expander_create, NULL,
		sizeof(FilterCompressorData_t), 0
	},

	{
		"Bitcrusher",
		"Bit loss;f=B;o=0;t=range;min=0;max=10;step=1;val=1",
		filter_bitcrusher_apply, filter_bitcrusher_debug, filter_bitcrusher_create, NULL,
		sizeof(FilterBitcrusherData_t), 0
	},

	{
		"Vibrato",
		"Delay;f=H;o=0;t=range;min=1;max=4999;step=1;val=10" PARAM_SEP
		"Frequency;f=B;o=2;t=range;min=1;max=10;step=1;val=1" PARAM_SEP
		"Wave Type;o=3" WAVE_TYPE_KV,
		filter_vibrato_apply, filter_vibrato_debug, filter_vibrato_create, NULL,
		sizeof(FilterVibratoData_t), 0
	},

	{
		"Tremolo",
		"Frequency;f=B;o=0;t=range;min=1;max=10;step=1;val=1" PARAM_SEP
		"Wave Type;o=1" WAVE_TYPE_KV PARAM_SEP
		"Depth;f=f;o=2;t=range;min=0;max=1;step=0.05;val=0.5",
		filter_tremolo_apply, filter_tremolo_debug, filter_tremolo_create, NULL,
		sizeof(FilterTremoloData_t), 0
	},

	{
		"Band-Pass",
		"Co-efficients;f=B;o=0;t=range;min=1;max=50;step=1;val=15" PARAM_SEP
		"Centre frequency;f=H;o=1;t=range;min=20;max=2500;step=1;val=1000" PARAM_SEP
		"Width;f=H;o=3;t=range;min=20;max=5000;step=2;val=500",
		filter_fir_apply, filter_bandpass_debug, filter_bandpass_create, filter_bandpass_mod,
		sizeof(FilterBandPassData_t), offsetof(FilterFIRBaseData_t, nCoefficients)
	},

	// {
	// 	"Oscillating BP",
	// 	"Frequency;f=f;o=2;t=range;min=1;max=10;step=1;val=1" PARAM_SEP
	// 	"Wave Type;o=6" WAVE_TYPE_KV PARAM_SEP
	// 	"Width;f=H;o=0;t=range;min=20;max=5000;step=2;val=500" PARAM_SEP
	// 	"Min Frequency;f=H;o=7;t=range;min=20;max=2500;step=1;val=50" PARAM_SEP
	// 	"Max Frequency;f=H;o=9;t=range;min=20;max=2500;step=1;val=1000",
	// 	filter_oscillating_bandpass_apply, filter_oscillating_bandpass_debug, filter_oscillating_bandpass_create, NULL,
	// 	sizeof(FilterOscillatingBandPassData_t), offsetof(FilterBandPassData_t, iWidth)
	// }
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
		dbg_printf("#%u: %s, apply=%p, debug=%p, create=%p, mod=%p, datasize=%u(%u private)\r\n", i, pFilter->pszName, (void *)pFilter->pfnApply, (void *)pFilter->pfnDebug, (void *)pFilter->pfnCreateCallback, (void *)pFilter->pfnModCallback, pFilter->nFilterDataSize, pFilter->nNonPublicDataSize);
	}

	dbg_printn("\r\n", -1);
}
#pragma GCC diagnostic pop
