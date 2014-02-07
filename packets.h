#ifndef _PACKETS_H_
#define _PACKETS_H_

typedef enum
{
	A2A_PROBE,			///< Board sends a probe packet on boot, waits for UI to respond with a probe
	U2B_RESET,			///< UI sends a packet to reset board
	B2U_PRINT,			///< Board debug prints to UI console
	B2U_FILTER_LIST,	///< Board sends available filters to UI
	U2B_FILTER_CHAIN,	///< UI is sending a full filter chain
	U2B_FILTER_MOD,		///< UI is changing a filter parameter

	// Must be last
	PACKET_TYPE_MAX,
} PacketType_e;

extern const char *g_ppszPacketTypes[];

void packet_probe_send(void);
void packet_probe_wait(void);
void packet_reset_wait(void);
void packet_print_send(const char *pszLine, size_t size);
void packet_filter_list_send(void);

#endif
