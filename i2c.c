#include <inttypes.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-pedantic"
#	include "lpc17xx_pinsel.h"
#	include "lpc17xx_i2c.h"
#pragma GCC diagnostic pop

#include "i2c.h"
#include "usbcon.h"
#include "dbg.h"


static int s_bI2CDebug = 0;


void i2c_init(void)
{
	usbcon_writef("Initialising I2C... ");

	PINSEL_CFG_Type PinCfg;
	PinCfg.Funcnum = 3;
	PinCfg.OpenDrain = 0;
	PinCfg.Portnum = 0;
	PinCfg.Pinmode = 0;

	PinCfg.Pinnum = 0; //SDA
	PINSEL_ConfigPin(&PinCfg);

	PinCfg.Pinnum = 1; //SCL
	PINSEL_ConfigPin(&PinCfg);

	I2C_Init(I2C_DEV, I2C_CLK);
	I2C_Cmd(I2C_DEV, ENABLE);

	usbcon_writef(ANSI_COLOR_GREEN "OK!\r\n" ANSI_COLOR_RESET);
}


int i2c_debug(int dbg)
{
	int bOldDebug = s_bI2CDebug;
	s_bI2CDebug = dbg;
	return bOldDebug;
}


uint32_t i2c_transfer(uint32_t iAddr, uint8_t *pTx, uint32_t nTxLen, uint8_t *pRx, uint32_t nRxLen)
{
	I2C_M_SETUP_Type i2cCfg;
	i2cCfg.tx_data = pTx;
	i2cCfg.tx_length = nTxLen;
	i2cCfg.rx_data = pRx;
	i2cCfg.rx_length = nRxLen;
	i2cCfg.sl_addr7bit = iAddr;
	i2cCfg.retransmissions_max = 3;

	if(s_bI2CDebug)
	{
		usbcon_writef("i2c_transfer @ 0x%lX\r\n", iAddr);

		// Debug transfer
		if(pTx)
		{
			usbcon_write("\t---> ", -1);

			for(uint32_t i = 0; i < nTxLen-1; ++i)
				usbcon_writef("0x%02X,", pTx[i]);

			usbcon_writef("0x%02X\r\n", pTx[nTxLen-1]);
		}
	}

	uint32_t status = I2C_MasterTransferData(I2C_DEV, &i2cCfg, I2C_TRANSFER_POLLING);

	if(i2cCfg.tx_count != nTxLen)
		dbg_warning("only transferred %lu of %lu bytes\r\n", i2cCfg.tx_count, nTxLen);

	if(s_bI2CDebug && pRx)
	{
		usbcon_write("\t<--- ", -1);

		for(uint32_t i = 0; i < nRxLen-1; ++i)
			usbcon_writef("0x%02X,", pRx[i]);

		usbcon_writef("0x%02X\r\n", pRx[nRxLen-1]);

		if(i2cCfg.rx_count != nRxLen)
			dbg_warning("receive buffer %lu bytes, only read %lu\r\n", nRxLen, i2cCfg.rx_count);
	}

	if(status != SUCCESS)
		dbg_warning("non-successful transfer status: %lu(%lx)\r\n", status, status);

	return status;
}


int i2c_probe_addr(uint32_t iAddr)
{
	uint8_t buf;

	I2C_M_SETUP_Type i2cCfg;
	i2cCfg.tx_data = NULL;
	i2cCfg.tx_length = 0;
	i2cCfg.rx_data = &buf;
	i2cCfg.rx_length = 1;
	i2cCfg.sl_addr7bit = iAddr;
	i2cCfg.retransmissions_max = 3;

	return I2C_MasterTransferData(I2C_DEV, &i2cCfg, I2C_TRANSFER_POLLING) == SUCCESS ? 1 : 0;
}


void i2c_scan(void)
{
	usbcon_writef("Scanning for I2C devices...\r\n");
	int nDevices = 0;

	for(int i = 0; i < (1<<7); ++i)
	{
		if(!i2c_probe_addr(i))
			continue;

		usbcon_writef("\tdevice at 0x%02X\r\n", i);
		nDevices++;
	}

	usbcon_writef("Found " ANSI_COLOR_GREEN "%d" ANSI_COLOR_RESET " I2C device(s)\r\n", nDevices);
}
