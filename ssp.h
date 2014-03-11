/*
 * ssp.c - SSP communication
 *
 * Defines several functions for communicating with SSP (e.g., to the SD card)
 *
 */

#ifndef _SSP_H_
#define _SSP_H_

#include <stdint.h>
#include <stdbool.h>

// LPC SSP device
#define SD_DEV LPC_SSP1

// SD card chip select pin
#define SD_CS_PIN 11


/*
 * SSPCommandFrame_t
 *
 * Structure of an SD command frame send over SSP.
 */
#pragma pack(push, 1)
typedef struct
{
	uint8_t index;		///< index of the command
	uint32_t argument;	///< argument data
	uint8_t crc;		///< CRC (can be 0 except for certain commands, see sd.c)
} SSPCommandFrame_t;
#pragma pack(pop)


void ssp_init(void);
uint8_t ssp_readwrite(uint8_t byte);
uint8_t ssp_read(void);

#endif
