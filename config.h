/*
 *	HAPR Project 2014
 *	Group 6 - Tom Bryant (TB) & Saul Rennison (SR)
 *
 *	File created by:	SR
 *	File modified by:	TB & SR
 *	File debugged by:	TB & SR
 *
 * config.h - Various constants
 */

#ifndef _CONFIG_H_
#define _CONFIG_H_

#define PI_F			3.14159265359f

// Sample rate
#define SAMPLE_RATE		10000

// Number of samples to hold in memory
#define BUFFER_SAMPLES	10000

// Peripheral extreme values
#define ADC_MAX_VALUE	((1<<12)-1)
#define DAC_MAX_VALUE	((1<<10)-1)
#define ADC_MID_POINT	((ADC_MAX_VALUE+1)/2)

// LEDs to use to indicate important states
#define LED_CLIP		0
#define LED_SLOW		1
#define LED_PASS_THRU	3

#endif
