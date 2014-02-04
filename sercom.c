/*
 * sercom.c - USB serial communication support
 *
 * Defines several functions for communication over USB serial.
 *
 */

#include <stdio.h>
#include <stdarg.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-pedantic"
#	include "lpc17xx_uart.h"
#	include "lpc17xx_pinsel.h"
#pragma GCC diagnostic pop

#include "sercom.h"
#include "dbg.h"


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

	// Wait to receive 'H'ello byte from the machine to proceed startup
	uint8_t ch = 0;

	do
	{
		UART_Receive((LPC_UART_TypeDef *)LPC_UART0, (uint8_t *)&ch, 1, BLOCKING);
	} while(ch != 'H');


	// Move cursor to 1,1, clear display and print initialised message
	dbg_printn("\x1b[;H\x1b[2J" ANSI_COLOR_RESET "Initialised USB console\r\n", -1);
}


/*
 * sercom_send
 *
 * Send a packet header followed by the packet data (in `pBuf`).
 */
void sercom_send(int packet_type, const uint8_t *pBuf, uint8_t size)
{
	dbg_assert(packet_type < PACKET_TYPE_MAX, "invalid packet type %d", packet_type);

	// Send packet header
	PacketHeader_t hdr = {
		.ident=PACKET_IDENT,
		.type=packet_type,
		.size=size
	};

	UART_Send((LPC_UART_TypeDef *)LPC_UART0, (uint8_t *)&hdr, sizeof(hdr), BLOCKING);

	// Send packet data
	UART_Send((LPC_UART_TypeDef *)LPC_UART0, (uint8_t *)pBuf, size, BLOCKING);
}
