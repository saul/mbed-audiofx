/*
 *	HAPR Project 2014
 *	Group 6 - Tom Bryant (TB) & Saul Rennison (SR)
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

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-pedantic"
#	include "lpc17xx_uart.h"
#	include "lpc17xx_pinsel.h"
#pragma GCC diagnostic pop

#include "sercom.h"
#include "dbg.h"

volatile bool g_bUARTLock = false;


static void startup_probe_wait(void)
{
	PacketHeader_t hdr;
	uint8_t *pPayload = NULL;

	for(;;)
	{
		if(!sercom_receive(&hdr, &pPayload))
			continue;

		dbg_printf("startup_probe_wait: received packet %u(%s)\r\n", hdr.type, g_ppszPacketTypes[hdr.type]);

		free(pPayload);

		if(hdr.type == A2A_PROBE)
			break;

		if(hdr.type == U2B_RESET)
		{
			NVIC_SystemReset();
			return;
		}
	}
}


/*
 * sercom_init
 *
 * Initialises the USB pins and UART config.
 */
void sercom_init(void)
{
	UART_CFG_Type UARTConfigStruct;
	UART_FIFO_CFG_Type UARTFIFOConfigStruct;
	PINSEL_CFG_Type PinCfg;

	/*
	 * Initialize UART pin connect
	 */
	PinCfg.Funcnum = 1;
	PinCfg.OpenDrain = 0;
	PinCfg.Pinmode = 0;

	// USB serial first
	PinCfg.Portnum = 0;
	PinCfg.Pinnum = 2;
	PINSEL_ConfigPin(&PinCfg);

	PinCfg.Pinnum = 3;
	PINSEL_ConfigPin(&PinCfg);

	/* Initialize UART Configuration parameter structure to default state:
	 * - baud rate = 9600bps
	 * - 8 data bit
	 * - 1 stop bit
	 * - None parity
	 */
	UART_ConfigStructInit(&UARTConfigStruct);

	/* Initialize FIFOConfigStruct to default state:
	 * - FIFO_DMAMode = DISABLE
	 * - FIFO_Level = UART_FIFO_TRGLEV0
	 * - FIFO_ResetRxBuf = ENABLE
	 * - FIFO_ResetTxBuf = ENABLE
	 * - FIFO_State = ENABLE
	 */
	UART_FIFOConfigStructInit(&UARTFIFOConfigStruct);

	// Initialize UART0 peripheral with given to corresponding parameter
	UART_Init((LPC_UART_TypeDef *)LPC_UART0, &UARTConfigStruct);

	// Initialize FIFO for UART0 peripheral
	UART_FIFOConfig((LPC_UART_TypeDef *)LPC_UART0, &UARTFIFOConfigStruct);

	// Enable UART Transmit
	UART_TxCmd((LPC_UART_TypeDef *)LPC_UART0, ENABLE);

	// Send a probe packet. If a machine is connected, it will send a probe back
	packet_probe_send();

	// Wait to receive probe from UI to proceed startup
	startup_probe_wait();

	// Move cursor to 1,1, clear display and print initialised message
	dbg_printn("\x1b[;H\x1b[2J" ANSI_COLOR_RESET "Initialised USB console\r\n", -1);
}


/*
 * sercom_send
 *
 * Send a packet header followed by the packet data (in `pBuf`).
 */
void sercom_send(PacketType_e packet_type, const uint8_t *pBuf, uint16_t size)
{
	dbg_assert(packet_type < PACKET_TYPE_MAX, "invalid packet type %d", packet_type);

	// Send packet header
	PacketHeader_t hdr = {
		.ident=PACKET_IDENT,
		.type=packet_type,
		.size=size
	};

	while(g_bUARTLock);

	g_bUARTLock = true;
	UART_Send((LPC_UART_TypeDef *)LPC_UART0, (uint8_t *)&hdr, sizeof(hdr), BLOCKING);

	if(!size)
	{
		g_bUARTLock = false;
		return;
	}

	dbg_assert(pBuf, "packet size > 0 but no payload supplied");

	// Send packet data
	UART_Send((LPC_UART_TypeDef *)LPC_UART0, (uint8_t *)pBuf, size, BLOCKING);
	g_bUARTLock = false;
}


PacketHeader_t *sercom_receive_nonblock(uint8_t **ppPayload)
{
	static PacketHeader_t hdr;
	static uint32_t nHeaderReceived = 0;

	// Have we received the entirety of a header?
	if(nHeaderReceived < sizeof(hdr))
	{
		uint8_t *pHdr = (uint8_t *)&hdr;

		while(g_bUARTLock);

		g_bUARTLock = true;
		nHeaderReceived += UART_Receive((LPC_UART_TypeDef *)LPC_UART0, (uint8_t *)&pHdr[nHeaderReceived], sizeof(hdr) - nHeaderReceived, NONE_BLOCKING);
		g_bUARTLock = false;

		if(nHeaderReceived != sizeof(hdr))
			return NULL;
	}

	// Reset header received count
	nHeaderReceived = 0;

	// Validate header ident
	if(hdr.ident != PACKET_IDENT)
	{
		dbg_warning("invalid packet identifier (%.4s)\r\n", (const char *)&hdr.ident);
		return NULL;
	}

	// Validate packet type
	if(hdr.type >= PACKET_TYPE_MAX)
	{
		dbg_warning("invalid packet type (%u)\r\n", hdr.type);
		return NULL;
	}

	// Read the packet payload
	if(hdr.size > 0)
	{
		if(!ppPayload)
		{
			dbg_warning("packet has payload (size=%u), but ppPayload is NULL\r\n", hdr.size);
			return NULL;
		}

		*ppPayload = malloc(hdr.size);
		dbg_assert(*ppPayload, "unable to allocate enough space for packet");

		uint32_t nBytes = UART_Receive((LPC_UART_TypeDef *)LPC_UART0, *ppPayload, hdr.size, BLOCKING);
		dbg_assert(nBytes == hdr.size, "failed to read payload (%lu of %u) from serial", nBytes, hdr.size);
	}

	return &hdr;
}


bool sercom_receive(PacketHeader_t *pHdr, uint8_t **ppPayload)
{
	dbg_assert(pHdr, "header must not be NULL");

	// Read a packet header from serial
	uint32_t nBytes = UART_Receive((LPC_UART_TypeDef *)LPC_UART0, (uint8_t *)pHdr, sizeof(*pHdr), BLOCKING);

	if(nBytes != sizeof(*pHdr))
	{
		dbg_warning("failed to read %u bytes from serial\r\n", sizeof(*pHdr));
		return false;
	}

	if(pHdr->ident != PACKET_IDENT)
	{
		dbg_warning("invalid packet identifier (%.4s)\r\n", (const char *)&pHdr->ident);
		return false;
	}

	if(pHdr->type >= PACKET_TYPE_MAX)
	{
		dbg_warning("invalid packet type (%u)\r\n", pHdr->type);
		return false;
	}

	// Read the packet payload
	if(pHdr->size > 0)
	{
		if(!ppPayload)
		{
			dbg_warning("packet has payload (size=%u), but ppPayload is NULL\r\n", pHdr->size);
			return false;
		}

		*ppPayload = malloc(pHdr->size);
		dbg_assert(*ppPayload, "unable to allocate enough space for packet");

		nBytes = UART_Receive((LPC_UART_TypeDef *)LPC_UART0, *ppPayload, pHdr->size, BLOCKING);
		dbg_assert(nBytes == pHdr->size, "failed to read payload (%lu of %u) from serial", nBytes, pHdr->size);
	}

	return true;
}
