/*
 *	HAPR Project 2014
 *	Group 6 - Tom Bryant (TB) & Saul Rennison (SR)
 *
 *	File modified from TB & SR Mini-Project work
 *
 * i2c.c - i2c functions
 *
 * Defines several functions for configuring and communicating on the i2c bus.
 */

#include <inttypes.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-pedantic"
#	include "lpc17xx_pinsel.h"
#	include "lpc17xx_i2c.h"
#pragma GCC diagnostic pop

#include "i2c.h"
#include "dbg.h"


static bool s_bI2CDebug = 0;


/*
 * i2c_init
 *
 * Initialises the i2c peripheral and its associated pins.
 */
void i2c_init(void)
{
	dbg_printf("Initialising I2C... ");

	PINSEL_CFG_Type PinCfg;
	PinCfg.Funcnum = 3;
	PinCfg.OpenDrain = 0;
	PinCfg.Portnum = 0;
	PinCfg.Pinmode = 0;

	PinCfg.Pinnum = 0; // SDA
	PINSEL_ConfigPin(&PinCfg);

	PinCfg.Pinnum = 1; // SCL
	PINSEL_ConfigPin(&PinCfg);

	I2C_Init(I2C_DEV, I2C_CLK);
	I2C_Cmd(I2C_DEV, ENABLE);

	dbg_printf(ANSI_COLOR_GREEN "OK!\r\n" ANSI_COLOR_RESET);
}


/*
 * i2c_debug
 *
 * Sets transfer debug flag. If set, all transfers are printed to console.
 */
bool i2c_debug(bool dbg)
{
	bool bOldDebug = s_bI2CDebug;
	s_bI2CDebug = dbg;
	return bOldDebug;
}


/*
 * i2c_transfer
 *
 * Transfers `nTxLen` bytes to `iAddr` and reads `nRxLen` bytes from the device
 * If `pTx` is NULL, no data is sent. If `pRx` is NULL, no data is read.
 *
 * @returns true if successful
 */
bool i2c_transfer(uint32_t iAddr, uint8_t *pTx, uint32_t nTxLen, uint8_t *pRx, uint32_t nRxLen)
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
		dbg_printf("i2c_transfer @ 0x%lX\r\n", iAddr);

		// Debug transfer
		if(pTx)
		{
			dbg_printn("\t---> ", -1);

			for(uint32_t i = 0; i < nTxLen-1; ++i)
				dbg_printf("0x%02X,", pTx[i]);

			dbg_printf("0x%02X\r\n", pTx[nTxLen-1]);
		}
	}

	// Transfer data
	Status status = I2C_MasterTransferData(I2C_DEV, &i2cCfg, I2C_TRANSFER_POLLING);

	// Did we not transfer as much as was requested?
	if(i2cCfg.tx_count != nTxLen)
		dbg_warning("only transferred %lu of %lu bytes\r\n", i2cCfg.tx_count, nTxLen);

	// Debug data read back
	if(s_bI2CDebug && pRx)
	{
		dbg_printn("\t<--- ", -1);

		if(i2cCfg.rx_count)
		{
			for(uint32_t i = 0; i < i2cCfg.rx_count-1; ++i)
				dbg_printf("0x%02X,", pRx[i]);

			dbg_printf("0x%02X\r\n", pRx[i2cCfg.rx_count-1]);
		}
	}

	// Failed to read back as much data as requested?
	if(i2cCfg.rx_count != nRxLen)
		dbg_warning("receive buffer %lu bytes, only read %lu\r\n", nRxLen, i2cCfg.rx_count);

	// Did the transfer fail?
	if(status != SUCCESS)
		dbg_warning("non-successful transfer\r\n");

	return status == SUCCESS;
}


/*
 * i2c_probe_addr
 *
 * Does a device exist at `iAddr`?
 *
 * @returns true if device exists at `iAddr`
 */
bool i2c_probe_addr(uint32_t iAddr)
{
	uint8_t buf;

	I2C_M_SETUP_Type i2cCfg;
	i2cCfg.tx_data = NULL;
	i2cCfg.tx_length = 0;
	i2cCfg.rx_data = &buf;
	i2cCfg.rx_length = 1;
	i2cCfg.sl_addr7bit = iAddr;
	i2cCfg.retransmissions_max = 3;

	return I2C_MasterTransferData(I2C_DEV, &i2cCfg, I2C_TRANSFER_POLLING) == SUCCESS;
}


/*
 * i2c_scan
 *
 * Prints all available i2c devices to console.
 */
void i2c_scan(void)
{
	dbg_printf("Scanning for I2C devices...\r\n");
	uint8_t nDevices = 0;

	// Iterate through range [0-128)
	for(uint8_t i = 0; i < (1<<7); ++i)
	{
		if(!i2c_probe_addr(i))
			continue;

		dbg_printf("\tdevice at 0x%02X\r\n", i);
		nDevices++;
	}

	dbg_printf("Found " ANSI_COLOR_GREEN "%u" ANSI_COLOR_RESET " I2C device(s)\r\n", nDevices);
}
