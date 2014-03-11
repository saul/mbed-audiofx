/*
 * dac.c - Digital -> Analog functions
 *
 * Defines several functions for configuring and reading from the DAC.
 * This implementation uses MBED Pin 18.
 *
 * MBED Pin mapping available:
 * 	http://www-users.cs.york.ac.uk/~pcc/MCP/MbedPins.html
 */

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-pedantic"
#	include "lpc17xx_dac.h"
#	include "lpc17xx_pinsel.h"
#pragma GCC diagnostic pop

#include "dac.h"


/*
 * dac_init
 *
 * Initialises the DAC peripheral.
 */
void dac_init(void)
{
	// MBED Pin 18
	PINSEL_CFG_Type pcfg;
	pcfg.Pinmode = 0;
	pcfg.Funcnum = 2;
	pcfg.Pinnum = 26;
	pcfg.Portnum = 0;

	PINSEL_ConfigPin(&pcfg);
	DAC_Init(DAC_DEV);
}


/*
 * dac_set
 *
 * Updates the signal value on pin 18.
 */
void dac_set(uint16_t val)
{
	DAC_UpdateValue(DAC_DEV, val);
}
