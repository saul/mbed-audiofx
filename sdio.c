#include "dbg.h"
#include "sd.h"
#include "ssp.h"
#include "fatfs/ff.h"
#include "fatfs/diskio.h"


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

	if(!(g_fSDStatus & SD_STATUS_READY))
		status |= STA_NOINIT;

	return status;
}


DRESULT disk_read(BYTE pdrv, BYTE* buff, DWORD sector, UINT count)
{
	dbg_assert(pdrv == 0, "invalid drive number");
	dbg_assert(count == 1, "multiple block read not supported");

	uint32_t addr = sector;

	if(!(g_fSDStatus & SD_STATUS_BLOCKADDR))
		addr *= SD_BLOCK_SIZE;

	uint8_t r1;
	sd_command(SDCMD_READ_SINGLE_BLOCK, addr, SD_RESP_R1, &r1, SD_TIMEOUT_INDEFINITE);

	if(r1 != 0)
	{
		dbg_warning("READ_SINGLE_BLOCK failed (%x)\r\n", r1);
		return RES_ERROR;
	}

	// Wait for data packet
	uint8_t token;
	while((token = ssp_readwrite(0xFF)) == 0xFF);

	// Check for error token
	if(!(token & (1<<5)))
	{
		dbg_warning("READ_SINGLE_BLOCK returned error token (%x)\r\n", token);
		return RES_ERROR;
	}

	// Read data block
	for(int i = 0; i < SD_BLOCK_SIZE; ++i)
		buff[i] = ssp_readwrite(0xFF);

	// Read CRC
	ssp_readwrite(0xFF);
	ssp_readwrite(0xFF);

	return RES_OK;
}


DRESULT disk_write(BYTE pdrv, const BYTE* buff, DWORD sector, UINT count)
{
	dbg_assert(pdrv == 0, "invalid drive number");
	return RES_ERROR;
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
