/*
 *	HAPR Project 2014
 *	Group 6 - Tom Bryant (TB) & Saul Rennison (SR)
 *
 *	File created by:	TB
 *	File modified by:	TB
 *	File debugged by:	TB
*/


#include "config.h"
#include "dbg.h"
#include "samples.h"
#include "distortion.h"


/*
 *	Bitcrusher applies distortion by losing some of the
 *	accuracy of each sample.
 *	Each sample has the lowest 'pData->bitLoss' samples set
 *	to zero, thus the higher 'pData->bitLoss', the greater
 *	the distortion.
 *
 *	inputs:
 *		input		signed 12 bit sample
 *		pUnknown	null pointer to FilterBitcrusherData_t data
 *
 *	output:
 *		signed 12 bit sample
 */
int16_t filter_bitcrusher_apply(int16_t input, void *pUnknown)
{
	const FilterBitcrusherData_t *pData = (const FilterBitcrusherData_t *)pUnknown;

	return (input >> pData->bitLoss) << (pData->bitLoss);
}


// Print bitcrusher parameter information to UI console
void filter_bitcrusher_debug(void *pUnknown)
{
	const FilterBitcrusherData_t *pData = (const FilterBitcrusherData_t *)pUnknown;
	dbg_printf("bitLoss=%u", pData->bitLoss);
}


// Set initial creation bitcrusher parameter values
void filter_bitcrusher_create(void *pUnknown)
{
	FilterBitcrusherData_t *pData = (FilterBitcrusherData_t *)pUnknown;
	pData->bitLoss = 1;
}
