/*
 * dac.c - Digital -> Analog functions
 *
 * Defines several functions for configuring and reading from the DAC.
 * This implementation uses MBED Pin 18.
 *
 * MBED Pin mapping available:
 * 	http://www-users.cs.york.ac.uk/~pcc/MCP/MbedPins.html
 */

#ifndef _DAC_H_
#define _DAC_H_

#define DAC_DEV LPC_DAC

void dac_init(void);
void dac_set(uint16_t val);

#endif
