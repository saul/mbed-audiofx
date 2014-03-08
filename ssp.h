#ifndef _SSP_H_
#define _SSP_H_

#include <stdint.h>
#include <stdbool.h>

#define SD_DEV LPC_SSP1
#define SD_CS_PIN 11


#pragma pack(push, 1)
typedef struct
{
	uint8_t index;
	uint32_t argument;
	uint8_t crc;
} SSPCommandFrame_t;
#pragma pack(pop)


void ssp_init(void);
uint8_t ssp_readwrite(uint8_t byte);

#endif
