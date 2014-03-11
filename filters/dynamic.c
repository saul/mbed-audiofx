/*
 *	HAPR Project 2014
 *	Group 6 - Tom Bryant (TB) & Saul Rennison (SR)
 *
 *	File created by:	TB
 *	File modified by:	TB
 *	File debugged by:	TB
 *
 *	dynamic.c
 *
 *	Defines functions to apply various dynamic effects to a sample.
 */


#include "dbg.h"
#include "samples.h"
#include "dynamic.h"
#include "config.h"


/*
 *	Noisegate filter takes an average of the amplitude over the last
 *	'pData->sensitivity' samples. If the average is lower than
 *	'pData->threshold', silence is returned. Otherwise, 'input' is
 *	returned.
 *
 *	inputs:
 *		input		signed 12 bit audio sample
 *		pUnknown	null pointer to a FilterNoiseGateData_t data structure
 *
 *	output:
 *		signed 12 bit audio sample
 */
int16_t filter_noisegate_apply(int16_t input, void *pUnknown)
{
	const FilterNoiseGateData_t *pData = (const FilterNoiseGateData_t *)pUnknown;

	uint16_t average = sample_get_average(pData->sensitivity);
	if(average < pData->threshold)
		return 0;
	else
		return input;
}


// Prints noise gate parameter information to UI console
void filter_noisegate_debug(void *pUnknown)
{
	const FilterNoiseGateData_t *pData = (const FilterNoiseGateData_t *)pUnknown;
	dbg_printf("sensitivity=%u, threshold=%u", pData->sensitivity, pData->threshold);
}


// Set noise gate initial creation values
void filter_noisegate_create(void *pUnknown)
{
	FilterNoiseGateData_t *pData = (FilterNoiseGateData_t *)pUnknown;
	pData->sensitivity = 25;
	pData->threshold = 200;
}


/*
 *	Compressor takes an average of the amplitude over the last
 *	'pData->sensitivity' samples. If the average is lower than
 *	'pData->threshold', the input is returned. Otherwise, the input
 *	is scaled using 'pData->scalar'.
 *
 *	inputs:
 *		input		signed 12 bit audio sample
 *		pUnknown	null pointer to a FilterCompressorData_t data structure
 *
 *	output:
 *		signed 12 bit audio sample
 */
int16_t filter_compressor_apply(int16_t input, void *pUnknown)
{
	const FilterCompressorData_t *pData = (const FilterCompressorData_t *)pUnknown;
	input = input;

	uint16_t average = sample_get_average(pData->sensitivity);
	if(average < pData->threshold)
		return input;

	input = input * pData->scalar;

	if(input < -ADC_MID_POINT)
		return  -ADC_MID_POINT;
	if(input > ADC_MID_POINT-1)
		return ADC_MID_POINT-1;
	return input;
}


// Prints compressor parameter information to UI console
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdouble-promotion"
void filter_compressor_debug(void *pUnknown)
{
	const FilterCompressorData_t *pData = (const FilterCompressorData_t *)pUnknown;
	dbg_printf("sensitivity=%u, threshold=%u, scalar=%f", pData->sensitivity, pData->threshold, pData->scalar);
}
#pragma GCC diagnostic pop


// Set compressor initial creation values
void filter_compressor_create(void *pUnknown)
{
	FilterCompressorData_t *pData = (FilterCompressorData_t *)pUnknown;
	pData->sensitivity = 25;
	pData->threshold = 200;
	pData->scalar = 0.8;
}


/*
 * 	Expander takes an average of the amplitude over the last
 * 	'pData->sensitivity' samples. If the average is higher than
 * 	'pData->threshold', the input is returned. Otherwise, the input
 * 	is scaled using 'pData->scalar'.
 *
 *  inputs:
 * 	input		signed 12 bit audio sample
 * 	pUnknown	null pointer to a FilterCompressorData_t data structure.
 * 				FilterCompressorData_t structure used as the parameters
 * 				needed for expander are shared with those for compressor
 *
 *	output:
 *	signed 12 bit audio sample
 */
int16_t filter_expander_apply(int16_t input, void *pUnknown)
{
	const FilterCompressorData_t *pData = (const FilterCompressorData_t *)pUnknown;

	uint16_t average = sample_get_average(pData->sensitivity);
	if(average > pData->threshold)
		return input;

	input = input * pData->scalar;

	if(input < -ADC_MID_POINT)
		return -ADC_MID_POINT;
	if(input > ADC_MID_POINT-1)
		return ADC_MID_POINT-1;
	return (input);
}


// Set expander initial creation values
void filter_expander_create(void *pUnknown)
{
	FilterCompressorData_t *pData = (FilterCompressorData_t *)pUnknown;
	pData->sensitivity = 25;
	pData->threshold = 200;
	pData->scalar = 1.5;
}
