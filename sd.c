#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-pedantic"
#	include "lpc17xx_gpio.h"
#pragma GCC diagnostic pop

#include "dbg.h"
#include "ssp.h"
#include "sd.h"
#include "ticktime.h"
#include "rtc.h"

// How to use MMC/SDC: http://elm-chan.org/docs/mmc/mmc_e.html
// FatFs: http://elm-chan.org/fsw/ff/00index_e.html
// SD card specifications: https://www.sdcard.org/downloads/pls/simplified_specs/part1_410.pdf


uint8_t g_fSDStatus = 0;


void sd_init(void)
{
	dbg_assert(!(g_fSDStatus & SD_STATUS_READY), "card already initialised");

	dbg_printf("Initialising SD card:\r\n");

	uint8_t r1 = 0;

	//-----------------------------------------------------
	// Send CMD0
	//-----------------------------------------------------
	dbg_printf("\tsending software reset... ");
	sd_cs(false);

	while(!(r1 & SD_R1_IN_IDLE_STATE))
	{
		if(!sd_command(SDCMD_GO_IDLE_STATE, 0, SD_RESP_R1, &r1, 500))
		{
			dbg_printf(ANSI_COLOR_RED "not found!\r\n" ANSI_COLOR_RESET);
			return;
		}
	}

	dbg_printf(ANSI_COLOR_GREEN "OK!\r\n" ANSI_COLOR_RESET);

	//-----------------------------------------------------
	// Send CMD8 with 0x000001AA
	//-----------------------------------------------------
	SDR7Response_t r7;

	dbg_printf("\tidentifying card... ");
	sd_command(SDCMD_SEND_IF_COND, 0x000001AA, SD_RESP_R7, &r7, SD_TIMEOUT_INDEFINITE);
	if(r7.result & SD_R1_ILLEGAL_COMMAND)
	{
		dbg_printf(ANSI_COLOR_RED "unsupported (SDCv1/MMC)\r\n" ANSI_COLOR_RESET);
		return;
	}

	uint16_t lower12 = r7.extra & ((1<<12) - 1);
	if(lower12 != 0x1AA)
	{
		dbg_printf(ANSI_COLOR_RED "rejected (0x%x)\r\n" ANSI_COLOR_RESET, lower12);
		return;
	}

	dbg_printf(ANSI_COLOR_GREEN "SDCv2\r\n" ANSI_COLOR_RESET);
	g_fSDStatus |= SD_STATUS_SDV2;

	//-----------------------------------------------------
	// Send ACMD41 with HCS flag (bit 30)
	//-----------------------------------------------------
	dbg_printf("\tinitialising card... ");

	do
	{
		sd_command(SDCMD_APP_CMD, 0, SD_RESP_R1, &r1, SD_TIMEOUT_INDEFINITE);
		sd_command(SDCMD_APP_SEND_OP_COND, 1<<30, SD_RESP_R1, &r1, SD_TIMEOUT_INDEFINITE);
	}
	while(r1 & SD_R1_IN_IDLE_STATE);

	dbg_printf(ANSI_COLOR_GREEN "OK!\r\n" ANSI_COLOR_RESET);

	//-----------------------------------------------------
	// Send CMD58 to check CCS (when set, IO is in block address, fixed to 512)
	//-----------------------------------------------------
	SDR7Response_t r3;

	dbg_printf("\tchecking card capacity info... ");
	sd_command(SDCMD_READ_OCR, 0, SD_RESP_R3, &r3, SD_TIMEOUT_INDEFINITE);

	if(r3.extra & (1<<30))
	{
		dbg_printf("block addressing\r\n");
		g_fSDStatus |= SD_STATUS_BLOCKADDR;
	}
	else
		dbg_printf("byte addressing\r\n");

	//-----------------------------------------------------
	// Send CMD16 to standardise block size
	//-----------------------------------------------------
	dbg_printf("\tsetting block size... ");
	sd_command(SDCMD_SET_BLOCKLEN, SD_BLOCK_SIZE, SD_RESP_R1, &r1, SD_TIMEOUT_INDEFINITE);
	if(r1 != 0)
	{
		dbg_printf(ANSI_COLOR_RED "failed (0x%x)\r\n" ANSI_COLOR_RESET, r1);
		return;
	}

	dbg_printf(ANSI_COLOR_GREEN "OK!\r\n" ANSI_COLOR_RESET);

	// Card initialised!
	g_fSDStatus |= SD_STATUS_READY;
}


void sd_cs(bool bHigh)
{
	if(bHigh)
		GPIO_SetValue(0, 1 << SD_CS_PIN);
	else
		GPIO_ClearValue(0, 1 << SD_CS_PIN);
}


static uint32_t swap32(uint32_t num)
{
	return ((num>>24) & 0x000000ff) |	// move byte 3 to byte 0
		   ((num>>8)  & 0x0000ff00) |	// move byte 2 to byte 1
		   ((num<<8)  & 0x00ff0000) |	// move byte 1 to byte 2
		   ((num<<24) & 0xff000000);	// move byte 0 to byte 3
}


void sd_send_command(uint8_t index, uint32_t argument)
{
	dbg_assert(index < 0x40, "invalid command index");

	// TODO: calculate CRC properly
	uint8_t crc = 0;

	if(index == SDCMD_GO_IDLE_STATE)
	{
		dbg_assert(argument == 0, "unexpected argument for CRC");
		crc = 0x94;
	}

	if(index == SDCMD_SEND_IF_COND)
	{
		dbg_assert(argument == 0x1AA, "unexpected argument for CRC");
		crc = 0x86;
	}

	SSPCommandFrame_t frame;
	frame.index = index | 0x40;
	frame.argument = swap32(argument);
	frame.crc = crc | 0x1;

	// Send each byte of the command frame
	for(uint8_t i = 0; i < sizeof(frame); ++i)
		ssp_readwrite(((uint8_t *)&frame)[i]);

#ifdef SSP_DEBUG_TX
	dbg_printn("{", 1);
	for(uint8_t i = 0; i < sizeof(frame); ++i)
	{
		uint8_t b = ((uint8_t*)&frame)[i];
		dbg_printf(" %02x", b);
	}
	dbg_printn(" } ", 3);
#endif
}


bool sd_command(uint8_t index, uint32_t argument, SDResponseType_e respType, void *pRespData, uint32_t ulTimeoutMsec)
{
	dbg_assert(pRespData, "NULL response data");

	sd_send_command(index, argument);

	// Keep reading data until we get an R1 response...
	uint32_t ulStartTick = time_tickcount();
	uint8_t data;
	do
	{
		data = ssp_read();
	}
	while(data == 0xFF && (ulTimeoutMsec == SD_TIMEOUT_INDEFINITE || time_tickcount() - ulStartTick < ulTimeoutMsec));

	// Did we timeout?
	if(data == 0xFF)
		return false;

	switch(respType)
	{
	case SD_RESP_R1:;
		uint8_t *pR1 = (uint8_t *)pRespData;
		*pR1 = data;
		break;

	case SD_RESP_R3:;
	case SD_RESP_R7:;
		SDR7Response_t *pR7 = (SDR7Response_t *)pRespData;
		pR7->result = data;

		// Read 4 more bytes (swap endian)
		for(int8_t i = sizeof(pR7->extra) - 1; i >= 0; --i)
			((uint8_t *)&pR7->extra)[i] = ssp_read();

		break;

	default:
		dbg_error("unsupported response type (%d)", respType);
	}

	return true;
}


DWORD get_fattime(void)
{
	RTC_TIME_Type rtcTime;
	RTC_GetFullTime(LPC_RTC, &rtcTime);

	return (rtcTime.SEC |
			(rtcTime.MIN << 5) |
			(rtcTime.HOUR << 11) |
			(rtcTime.DOM << 16) |
			(rtcTime.MONTH << 21) |
			((rtcTime.YEAR - 1980) << 25));
}


void fs_init(void)
{
	FRESULT res = f_mount(&g_fs, "", 1);

	dbg_printf("Mounting file system... ");

	if(res)
	{
		dbg_printf(ANSI_COLOR_RED "failed (%d)\r\n" ANSI_COLOR_RESET, res);
		return;
	}

	dbg_printf(ANSI_COLOR_GREEN "OK!\r\n" ANSI_COLOR_RESET);
}
