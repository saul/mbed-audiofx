/*
 * filters.c - Audio effect filters
 */

#include <stdlib.h>
#include <stdint.h>

#include "dbg.h"
#include "filters.h"
#include "filters/delay.h"


 /*
  * g_pFilters
  *
  * Global list of filter types.
  */
Filter_t g_pFilters[] = {
	{"Delay", "Simple delay filter", "delay=H", filter_delay_apply, filter_delay_debug, NULL, sizeof(FilterDelayData_t)},
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
	dbg_printf(" === filter_debug (%u filters) ===\r\n", NUM_FILTERS);

	for(uint8_t i = 0; i < NUM_FILTERS; ++i)
	{
		const Filter_t *pFilter = &g_pFilters[i];
		dbg_printf("\r\n#%02u: %s: %s\r\n", i, pFilter->pszName, pFilter->pszDescription);
		dbg_printf("     apply=%p\r\n", (void *)pFilter->pfnApply);
		dbg_printf("     debug=%p\r\n", (void *)pFilter->pfnDebug);
		dbg_printf("     validate=%p\r\n", (void *)pFilter->pfnValidate);
		dbg_printf("     privateDataSize=%u\r\n", pFilter->nPrivateDataSize);
	}

	dbg_printn("\r\n", -1);
}
#pragma GCC diagnostic pop
