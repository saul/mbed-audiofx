/*
 * sercom.c - USB serial communication support
 *
 * Defines several functions for communication over USB serial.
 *
 */

#ifndef _SERCOM_H_
#define _SERCOM_H_

// little-endian "MBED"
#define PACKET_IDENT (('D' << 24) | ('E' << 16) | ('B' << 8) | 'M')

#pragma pack(push, 1)
typedef struct
{
	uint32_t ident;
	uint8_t type;
	uint8_t size;
} PacketHeader_t;
#pragma pack(pop)

enum PacketType_e
{
	B2U_PRINT,			///< Board debug prints to UI console
	B2U_FILTER_LIST,	///< Board sends available filters to UI
	U2B_FILTER_CHAIN,	///< UI is sending a full filter chain
	U2B_FILTER_MOD,		///< UI is changing a filter parameter

	// Must be last
	PACKET_TYPE_MAX,
};

void sercom_init(void);
void sercom_send(int packet_type, const uint8_t *pBuf, uint8_t size);

#endif
