/*
 *	HAPR Project 2014
 *	Group 6 - Tom Bryant (TB) & Saul Rennison (SR)
 *	Saul Rennison Individual Part
 *
 *	File created by:	SR
 *	File modified by:	TB & SR
 *	File debugged by:	TB & SR
 *
 * sercom.c - USB serial communication support
 *
 * Defines several functions for communication over USB serial.
 *
 */

#ifndef _SERCOM_H_
#define _SERCOM_H_

#include <stdbool.h>
#include "packets.h"

// little-endian "MBED"
#define PACKET_IDENT (('D' << 24) | ('E' << 16) | ('B' << 8) | 'M')

extern volatile bool g_bUARTLock;

void sercom_init(void);
void sercom_send(PacketType_e packet_type, const uint8_t *pBuf, uint16_t size);
PacketHeader_t *sercom_receive_nonblock(uint8_t **ppPayload);
bool sercom_receive(PacketHeader_t *pHdr, uint8_t **ppPayload);

#endif
