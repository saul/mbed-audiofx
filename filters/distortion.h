/*
 *	HAPR Project 2014
 *	Group 6 - Tom Bryant (TB) & Saul Rennison (SR)
 *
 *	File created by:	TB
 *	File modified by:	TB
 *	File debugged by:	TB
*/


#ifndef _FILTER_DISTORTION_H_
#define _FILTER_DISTORTION_H_


// Structure to hold bitcrusher parameter information
typedef struct
{
	uint8_t bitLoss;	// between 0 and 10: number of bits of sensitivity to lose
} FilterBitcrusherData_t;


int16_t filter_bitcrusher_apply(int16_t input, void *pUnknown);
void filter_bitcrusher_debug(void *pUnknown);
void filter_bitcrusher_create(void *pUnknown);

#endif
