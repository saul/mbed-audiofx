#include "ssp.h"
#include "ssp.h"

// How to use MMC/SDC: http://elm-chan.org/docs/mmc/mmc_e.html
// FatFs: http://elm-chan.org/fsw/ff/00index_e.html
// ssp.h documentation: http://www-module.cs.york.ac.uk/hapr/resources/mbed_resources/CMSIS/drivers/html/group__SSP__Public__Functions.html#ga27795785a9e9370ea2c17d65d3e2fcd6
// Pin mapping: http://www-users.cs.york.ac.uk/~pcc/MCP/MbedPins.html
// SD card specifications: https://www.sdcard.org/downloads/pls/simplified_specs/part1_410.pdf



#pragma pack(push, 1)
typedef struct
{
	uint8_t index;
	uint32_t argument;
	uint8_t crc;
} SSPCommandFrame_t;
#pragma pack(pop)

void ssp_init(void)
{
	dbg_printf("Initialising SSP... ");

	// Configure SSP pins
	PINSEL_CFG_Type pinConfig;
	pinConfig.OpenDrain = PINSEL_PINMODE_NORMAL;
	pinConfig.Pinmode = PINSEL_PINMODE_PULLUP;
	pinConfig.Portnum = 0;
	pinConfig.Funcnum = 2;

	// SDO pin
	pinConfig.Pinnum = 9;
	PINSEL_ConfigPin(&pinConfig);

	// SDI pin
	pinConfig.Pinnum = 8;
	PINSEL_ConfigPin(&pinConfig);

	// CLK pin
	pinConfig.Pinnum = 7;
	PINSEL_ConfigPin(&pinConfig);

	// Chip select pin
	pinConfig.Funcnum = 0;
	pinConfig.Pinnum = SD_CS_PIN;
	PINSEL_ConfigPin(&pinConfig);
	GPIO_SetDir(0, 1 << SD_CS_PIN, 1);

	// Initialise SSP
	SSP_CFG_Type sspConfig;
	SSP_ConfigStructInit(&sspConfig);
	sspConfig.ClockRate = 200 * 1000; // 200kHz (max 25MHz for SDC in most cases)

	SSP_Init(SD_DEV, &sspConfig);
	SSP_Cmd(SD_DEV, ENABLE);

	dbg_printf(ANSI_COLOR_GREEN "OK!\r\n" ANSI_COLOR_RESET);

	sd_init();
}


void sd_init(void)
{
	dbg_printf("Identifying SD card... ");

	sd_cs(true);
	time_sleep(10);

	uint32_t ulStartTick = time_tickcount();
	uint8_t byte = 0;
	uint32_t ulReadyTimeout = 5000;

	for(uint8_t byte = 0; byte != 0xFF && time_tickcount() - ulStartTick < ulReadyTimeout; byte = SSP_ReceiveData(SD_DEV))
		;

	if(byte == 0xFF)
	{
		dbg_printf(ANSI_COLOR_GREEN "OK!\r\n" ANSI_COLOR_RESET);
	}
	else
	{
		dbg_printf(ANSI_COLOR_RED "timeout!\r\n" ANSI_COLOR_RESET);
	}
}


void sd_cs(bool bHigh)
{
	if(bHigh)
		GPIO_SetValue(0, 1 << SD_CS_PIN);
	else
		GPIO_ClearValue(0, 1 << SD_CS_PIN);
}


void ssp_send_command(uint8_t index, uint32_t argument)
{
	// TODO: calculate CRC
	uint8_t crc = 0;

	SSPCommandFrame_t frame;
	frame.index = (1<<6) | (index & ((1<<6) - 1));
	frame.argument = argument;
	frame.crc = crc | 1;

	// Send each byte of the command frame
	for(uint8_t i < sizeof(frame); ++i)
		SSP_SendData(SD_DEV, ((uint8_t*)&frame)[i]);
}


void ssp_read_response(void)
{
	uint8_t byte = SSP_ReceiveData(SD_DEV);
	if(byte & (1<<7))
		return;
}
