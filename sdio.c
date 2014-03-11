/*
 * sdio.c - Interface between FatFS and the SD card.
 */

#include "dbg.h"
#include "sd.h"
#include "ssp.h"
#include "fatfs/ff.h"
#include "fatfs/diskio.h"


// Global FatFS workspace
FATFS g_fs;


DSTATUS disk_initialize(BYTE pdrv)
{
	dbg_assert(pdrv == 0, "invalid drive number");

	if(!(g_fSDStatus & SD_STATUS_READY))
		sd_init();

	return disk_status(pdrv);
}


DSTATUS disk_status(BYTE pdrv)
{
	dbg_assert(pdrv == 0, "invalid drive number");
	DSTATUS status = 0;

	// If the SD card is not ready define STA_NOINIT
	if(!(g_fSDStatus & SD_STATUS_READY))
		status |= STA_NOINIT;

	return status;
}


DRESULT disk_read(BYTE pdrv, BYTE* buff, DWORD sector, UINT count)
{
	dbg_assert(pdrv == 0, "invalid drive number");
	dbg_assert(count == 1, "multiple block read not supported");

	uint32_t addr = sector;

	// If we're byte addressing, multiply by blocksize
	if(!(g_fSDStatus & SD_STATUS_BLOCKADDR))
		addr *= SD_BLOCK_SIZE;

	// Send read block command
	uint8_t r1;
	sd_command(SDCMD_READ_SINGLE_BLOCK, addr, SD_RESP_R1, &r1, SD_TIMEOUT_INDEFINITE);

	if(r1 != 0)
	{
		dbg_warning("READ_SINGLE_BLOCK failed (%x)\r\n", r1);
		return RES_ERROR;
	}

	// Wait for data packet
	uint8_t token;
	while((token = ssp_read()) == 0xFF);

	// Check for error token
	if(!(token & (1<<5)))
	{
		dbg_warning("READ_SINGLE_BLOCK returned error token (%x)\r\n", token);
		return RES_ERROR;
	}

	// Read data block
	for(int i = 0; i < SD_BLOCK_SIZE; ++i)
		buff[i] = ssp_read();

	// Read CRC
	ssp_read();
	ssp_read();

	return RES_OK;
}


DRESULT disk_write(BYTE pdrv, const BYTE* buff, DWORD sector, UINT count)
{
	dbg_assert(pdrv == 0, "invalid drive number");
	dbg_assert(count == 1, "multiple block write not supported");

	uint32_t addr = sector;

	// If we're byte addressing, multiply by blocksize
	if(!(g_fSDStatus & SD_STATUS_BLOCKADDR))
		addr *= SD_BLOCK_SIZE;

	// Send write block command
	uint8_t r1;
	sd_command(SDCMD_WRITE_BLOCK, addr, SD_RESP_R1, &r1, SD_TIMEOUT_INDEFINITE);

	if(r1 != 0)
	{
		dbg_warning("WRITE_BLOCK failed (%x)\r\n", r1);
		return RES_ERROR;
	}

	// Write a byte before the data packet
	ssp_read();

	// Write data packet token for CMD24
	ssp_readwrite(~0x1);

	for(uint16_t i = 0; i < SD_BLOCK_SIZE; ++i)
		ssp_readwrite(buff[i]);

	// Write empty CRC
	ssp_readwrite(0x0);
	ssp_readwrite(0x0);

	// Read data response
	uint8_t iDataResponse = ssp_read() & 0x1F;

	if(iDataResponse != 0x5)
	{
		dbg_warning("unexpected data response (%x)", iDataResponse);
		return RES_ERROR;
	}

	// Wait until card not busy
	while(ssp_read() != 0xFF);

	return RES_OK;
}


DRESULT disk_ioctl(BYTE pdrv, BYTE cmd, void* buff)
{
	dbg_assert(pdrv == 0, "invalid drive number");

	switch(cmd)
	{
	case CTRL_SYNC:
		return RES_OK;

	case GET_SECTOR_COUNT:
		dbg_error("disk_ioctl GET_SECTOR_COUNT not supported!");
		return RES_PARERR;

	case GET_SECTOR_SIZE:
	case GET_BLOCK_SIZE:
		*((WORD *)buff) = SD_BLOCK_SIZE;
		return RES_OK;

	case CTRL_ERASE_SECTOR:
		dbg_error("disk_ioctl CTRL_ERASE_SECTOR not supported!");
		return RES_PARERR;
	}

	dbg_warning("unknown command (%d)\r\n", cmd);
	return RES_PARERR;
}
