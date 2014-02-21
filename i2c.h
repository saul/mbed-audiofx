#ifndef _I2C_H_
#define _I2C_H_

#include <stdbool.h>
#include "lpc_types.h"

#define I2C_DEV LPC_I2C1
#define I2C_CLK 100000

void i2c_init(void);
bool i2c_transfer(uint32_t iAddr, uint8_t *pTx, uint32_t nTxLen, uint8_t *pRx, uint32_t nRxLen);
bool i2c_probe_addr(uint32_t iAddr);
void i2c_scan(void);
bool i2c_debug(bool dbg);

#endif
