/*
 *	HAPR Project 2014
 *	Group 6 - Tom Bryant (TB) & Saul Rennison (SR)
 *
 *	File modified from TB & SR Mini-Project work
 *
 * adc.c - Analog -> Digital functions
 *
 * Defines several functions for configuring and reading from the ADC.
 *
 * MBED Pin mapping available:
 * 	http://www-users.cs.york.ac.uk/~pcc/MCP/MbedPins.html
 */

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-pedantic"
#	include "lpc17xx_pinsel.h"
#pragma GCC diagnostic pop

#include "config.h"
#include "adc.h"
#include "dbg.h"


/*
 * adc_init
 *
 * Initialises the ADC peripheral at a rate of `rate` Hz
 */
void adc_init(uint32_t rate)
{
	ADC_Init(ADC_DEV, rate);
}


/*
 * adc_config
 *
 * Configures the pins and ADC peripheral for a specific channel.
 */
void adc_config(uint8_t chan, bool bEnable)
{
	dbg_assert(chan < ADC_NUM_CHANNELS, "invalid channel (%u, max=%u)", chan, ADC_NUM_CHANNELS-1);

	PINSEL_CFG_Type adc_pcfg;
	adc_pcfg.OpenDrain = 0;
	adc_pcfg.Pinmode = 0;
	adc_pcfg.Pinnum = 0;
	adc_pcfg.Portnum = 0;
	adc_pcfg.Funcnum = 1;

	switch(chan)
	{
	case 0: // MBED Pin 15
		adc_pcfg.Pinnum = 23;
		break;
	case 1: // MBED Pin 16
		adc_pcfg.Pinnum = 24;
		break;
	case 2: // MBED Pin 17
		adc_pcfg.Pinnum = 25;
		break;
	case 3: // MBED Pin 18 (also DAC@funcnum 2, mutually exclusive)
		adc_pcfg.Pinnum = 26;
		break;
	case 4: // MBED Pin 19 (audio socket on UoY host board)
		adc_pcfg.Pinnum = 30;
		adc_pcfg.Portnum = 1;
		adc_pcfg.Funcnum = 3;
		break;
	case 5: // MBED Pin 20
		adc_pcfg.Pinnum = 31;
		adc_pcfg.Portnum = 1;
		adc_pcfg.Funcnum = 3;
		break;
	default:
		// Unreachable
		return;
	}

	if(bEnable)
		PINSEL_ConfigPin(&adc_pcfg);

	ADC_ChannelCmd(ADC_DEV, chan, bEnable ? ENABLE : DISABLE);
}


/*
 * adc_start
 *
 * Starts the ADC peripheral in a specific mode.
 */
void adc_start(uint8_t mode)
{
	ADC_StartCmd(ADC_DEV, mode);
}


/*
 * adc_read
 *
 * Read a 12-bit value from the ADC. If there is no signal on `chan`, this
 * function returns 0.
 */
uint16_t adc_read(uint8_t chan)
{
	dbg_assert(chan < ADC_NUM_CHANNELS, "invalid channel (%u, max=%u)", chan, ADC_NUM_CHANNELS-1);

	uint16_t val = ADC_ChannelGetData(ADC_DEV, chan);

	// The ADC floats high. Return 0 if we're clipped
	if(val == ADC_MAX_VALUE)
		val = 0;

	return val;
}


/*
 * adc_burst_config
 *
 * Enable burst mode for the ADC peripheral.
 */
void adc_burst_config(bool bEnable)
{
	ADC_BurstCmd(ADC_DEV, bEnable);
}


/*
 * adc_status
 *
 * Get ADC channel done status from ADC data register.
 *
 * @returns SET if done, RESET if not
 */
FlagStatus adc_status(uint8_t chan)
{
	dbg_assert(chan < ADC_NUM_CHANNELS, "invalid channel (%u, max=%u)", chan, ADC_NUM_CHANNELS-1);
	return ADC_ChannelGetStatus(ADC_DEV, chan, 1);
}
