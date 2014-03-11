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

#ifndef _ADC_H_
#define _ADC_H_

#include <stdbool.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-pedantic"
#	include "lpc17xx_adc.h"
#pragma GCC diagnostic pop

// Number of ADC channels available
// In the functions below, `chan` must be in the range [0..ADC_NUM_CHANNELS)
#define ADC_NUM_CHANNELS 6

// LPC ADC device
#define ADC_DEV LPC_ADC

void adc_init(uint32_t rate);
void adc_config(uint8_t chan, bool bEnable);
void adc_start(uint8_t mode);
uint16_t adc_read(uint8_t chan);
void adc_burst_config(bool bEnable);
FlagStatus adc_status(uint8_t chan);

#endif
