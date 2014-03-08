#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-pedantic"
#	include "lpc17xx_pinsel.h"
#	include "lpc17xx_gpio.h"
#	include "lpc17xx_ssp.h"
#pragma GCC diagnostic pop

#include "dbg.h"
#include "ssp.h"
#include "sd.h"

// ssp.h documentation: http://www-module.cs.york.ac.uk/hapr/resources/mbed_resources/CMSIS/drivers/html/group__SSP__Public__Functions.html#ga27795785a9e9370ea2c17d65d3e2fcd6
// Pin mapping: http://www-users.cs.york.ac.uk/~pcc/MCP/MbedPins.html

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

	// SDI pins
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
	sd_cs(true);

	// Initialise SSP
	SSP_CFG_Type sspConfig;
	SSP_ConfigStructInit(&sspConfig);
	sspConfig.ClockRate = 200 * 1000; // 200kHz (max 25MHz for SDC in most cases)

	SSP_Init(SD_DEV, &sspConfig);
	SSP_Cmd(SD_DEV, ENABLE);

	dbg_printf(ANSI_COLOR_GREEN "OK!\r\n" ANSI_COLOR_RESET);
}


uint8_t ssp_readwrite(uint8_t byte)
{
	// Wait for device to be ready
	while(SSP_GetStatus(SD_DEV, SSP_STAT_BUSY) == SET);
	SSP_SendData(SD_DEV, byte);

	// Wait for data to send
	while(SSP_GetStatus(SD_DEV, SSP_STAT_RXFIFO_NOTEMPTY) == RESET);
	return SSP_ReceiveData(SD_DEV);
}
