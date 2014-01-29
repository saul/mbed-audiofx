#include <stdlib.h>
#include <stdint.h>

#include "usbcon.h"
#include "filters.h"
#include "filters/delay.h"


Filter_t g_pFilters[] = {
	{"Delay", "Simple delay filter", filter_delay_apply, filter_delay_debug, NULL, sizeof(FilterDelayData_t)},
};


#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-pedantic" // ignore "function pointer -> void pointer" error
void filter_debug(void)
{
	uint8_t nFilters = sizeof(g_pFilters)/sizeof(g_pFilters[0]);
	usbcon_writef(" === filter_debug (%u filters) ===\r\n", nFilters);

	for(uint8_t i = 0; i < nFilters; ++i)
	{
		const Filter_t *pFilter = &g_pFilters[i];
		usbcon_writef("\r\n#%02u: %s: %s\r\n", i, pFilter->pszName, pFilter->pszDescription);
		usbcon_writef("     apply=%p\r\n", (void *)pFilter->pfnApply);
		usbcon_writef("     debug=%p\r\n", (void *)pFilter->pfnDebug);
		usbcon_writef("     validate=%p\r\n", (void *)pFilter->pfnValidate);
		usbcon_writef("     privateDataSize=%u\r\n", pFilter->nPrivateDataSize);
	}

	usbcon_write("\r\n", -1);
}
#pragma GCC diagnostic pop