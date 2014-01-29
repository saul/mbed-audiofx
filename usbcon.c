/*
 * usbcon.c - USB console support
 *
 * Defines several functions for very basic terminal emulation via USB.
 *
 * To interact with the console, use `screen /dev/ttyACM0`
 */

#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-pedantic"
#	include "lpc17xx_uart.h"
#	include "lpc17xx_pinsel.h"
#pragma GCC diagnostic pop

#include "usbcon.h"
#include "dbg.h"
#include "ticktime.h"


/*
 * usbcon_init
 *
 * Initialises the USB pins and UART config.
 */
void usbcon_init(void)
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

	// Move cursor to 1,1, clear display and print initialised message
	usbcon_write("\x1b[;H\x1b[2J" ANSI_COLOR_RESET "Initialised USB console\r\n", -1);
}


/*
 * usbcon_write
 *
 * Writes a string to the console. If length < 0, the number of characters to
 * write is taken as the length of `buf`.
 *
 * @returns number of characters written
 */
int usbcon_write(const char *buf, int length)
{
	if(length < 0)
		length = strlen(buf);

	return UART_Send((LPC_UART_TypeDef *)LPC_UART0, (uint8_t *)buf, length, BLOCKING);
}


int usbcon_writef(const char *format, ...)
{
	va_list args;
	va_start(args, format);

	char buf[128];
	vsnprintf(buf, sizeof(buf), format, args);

	va_end(args);

	return usbcon_write(buf, -1);
}


/*
 * usbcon_read
 *
 * Reads `length` bytes from console into `buf`.
 *
 * @returns number of characters read
 */
int usbcon_read(char *buf, int length)
{
	return UART_Receive((LPC_UART_TypeDef *)LPC_UART0, (uint8_t *)buf, length, BLOCKING);
}


/*
 * usbcon_readline
 *
 * Reads all characters typed into the console until \r. The returned string
 * does not contain the \r character.
 *
 * @returns pointer to static buffer of line read
 */
const char *usbcon_readline(void)
{
	static char line[64];
	size_t i = 0;

	while(i < sizeof(line))
	{
		char ch;

		// Read in a character from the USB console (note we need to check that
		// the return value is non-zero as the read call may timeout)
		if(usbcon_read(&ch, 1) != 1)
			continue;

		// ANSI escape code
		if(ch == 0x1b)
		{
			// TODO: support ANSI terminal cursor keys, function keys, delete,
			// etc. At the moment we just ignore all bytes that come
			// immediately after an ANSI escape code
			do
			{
				time_sleep(10);
			} while(UART_Receive((LPC_UART_TypeDef *)LPC_UART0, (uint8_t *)&ch, 1, NONE_BLOCKING));

			continue;
		}

		if(ch == '\r')
			break;

		// Handle backspacing
		// TODO: fix backspacing after a tab
		if(ch == '\b')
		{
			if(i > 0)
			{
				usbcon_write("\b \b", 3);
				i--;
			}

			continue;
		}

		if(i < (sizeof(line) - 1))
		{
			line[i] = ch;
			usbcon_write(&line[i++], 1);
		}
	}

	usbcon_write("\r\n", 2);

	// NUL terminate the line
	if(i == sizeof(line)) i--;
	line[i] = '\0';

	return line;
}

