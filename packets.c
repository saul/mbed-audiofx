#include <string.h>
#include <stdint.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-pedantic"
#	include "LPC17xx.h"
#	include "lpc_types.h"
#pragma GCC diagnostic pop

#include "sercom.h"
#include "filters.h"
#include "bytebuffer.h"
#include "dbg.h"


const char *g_ppszPacketTypes[] = {
	"A2A_PROBE",
	"U2B_RESET",
	"B2U_PRINT",
	"B2U_FILTER_LIST",
	"U2B_FILTER_CHAIN",
	"U2B_FILTER_MOD",
};


// TODO: fix this static assert
/*
static void constraints(void)
{
	// Ensure g_ppszPacketTypes matches the size of
	STATIC_ASSERT((sizeof(g_ppszPacketTypes)/sizeof(g_ppszPacketTypes[0])) == PACKET_TYPE_MAX);
}
*/


void packet_probe_send(void)
{
	sercom_send(A2A_PROBE, NULL, 0);
}


void packet_reset_wait(void)
{
	PacketHeader_t hdr;
	uint8_t *pPayload;

	for(;;)
	{
		if(!sercom_receive(&hdr, &pPayload))
			continue;

		if(hdr.type == U2B_RESET)
			NVIC_SystemReset();
	}
}


void packet_print_send(const char *pszLine, size_t size)
{
	sercom_send(B2U_PRINT, (const uint8_t *)pszLine, size);
}


void packet_filter_list_send(void)
{
	byte_buffer *buf = bb_new_default(true);
	bb_put(buf, NUM_FILTERS);

	for(size_t i = 0; i < NUM_FILTERS; ++i)
	{
		const Filter_t *pFilter = &g_pFilters[i];
		bb_put_string(buf, pFilter->pszName);
		bb_put_string(buf, pFilter->pszDescription);
		bb_put_string(buf, pFilter->pszParamFormat);
	}

	sercom_send(B2U_FILTER_LIST, buf->buf, buf->pos);
	bb_free(buf);
}
