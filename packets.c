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
#include "ticktime.h"


const char *g_ppszPacketTypes[] = {
	"A2A_PROBE",
	"U2B_RESET",
	"B2U_PRINT",
	"B2U_FILTER_LIST",
	"U2B_FILTER_CREATE",
	"U2B_FILTER_DELETE",
	"U2B_FILTER_FLAG",
	"U2B_FILTER_MOD",
};


PacketHandler_t g_ppfnPacketHandlers[] = {
	packet_reset_receive, // A2A_PROBE
	packet_reset_receive, // U2B_RESET
	NULL, // B2U_PRINT
	NULL, // B2U_FILTER_LIST
	packet_filter_create_receive, // U2B_FILTER_CREATE
	packet_filter_delete_receive, // U2B_FILTER_DELETE
	packet_filter_flag_receive, // U2B_FILTER_FLAG
	packet_filter_mod_receive, // U2B_FILTER_MOD
};


#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-pedantic"
void packet_static_assertions(void)
{
	_Static_assert(sizeof(g_ppszPacketTypes)/sizeof(g_ppszPacketTypes[0]) == PACKET_TYPE_MAX, "g_ppszPacketTypes size does not match number of packets");
	_Static_assert(sizeof(g_ppfnPacketHandlers)/sizeof(g_ppfnPacketHandlers[0]) == PACKET_TYPE_MAX, "g_ppfnPacketHandlers size does not match number of packets");
}
#pragma GCC diagnostic pop


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

		// If we receive a probe or reset packet, reset the board
		if(hdr.type == U2B_RESET || hdr.type == A2A_PROBE)
			packet_reset_receive(&hdr, pPayload);

		free(pPayload);
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
		bb_put_string(buf, pFilter->pszParamFormat);
	}

	sercom_send(B2U_FILTER_LIST, buf->buf, buf->pos);
	bb_free(buf);
}


void packet_loop(void)
{
	PacketHeader_t hdr;
	uint8_t *pPayload;

	if(!sercom_receive(&hdr, &pPayload))
		return;

	PacketHandler_t *pHandler = &g_ppfnPacketHandlers[hdr.type];

	if(pHandler)
		(*pHandler)(&hdr, pPayload);
	else
		dbg_warning("Received packet (%s) that has no handler!\r\n", g_ppszPacketTypes[hdr.type]);

	free(pPayload);
}


void packet_reset_receive(const PacketHeader_t *pHdr, const uint8_t *pPayload)
{
	dbg_printf("Received %s packet, system rebooting in 1 second...\r\n", g_ppszPacketTypes[pHdr->type]);
	time_sleep(1000);

	NVIC_SystemReset();
}


void packet_filter_create_receive(const PacketHeader_t *pHdr, const uint8_t *pPayload)
{

}


void packet_filter_delete_receive(const PacketHeader_t *pHdr, const uint8_t *pPayload)
{

}


void packet_filter_flag_receive(const PacketHeader_t *pHdr, const uint8_t *pPayload)
{

}


void packet_filter_mod_receive(const PacketHeader_t *pHdr, const uint8_t *pPayload)
{

}
