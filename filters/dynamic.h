/*
 *	HAPR Project 2014
 *	Group 6 - Tom Bryant (TB) & Saul Rennison (SR)
 *
 *	File created by:	TB
 *	File modified by:	TB
 *	File debugged by:	TB
 */


#ifndef _FILTER_DYNAMIC_H_
#define _FILTER_DYNAMIC_H_


// Structure used to hold noisegate data
typedef struct
{
	uint16_t sensitivity;	///< number of samples over which to take an average
	uint16_t threshold;		///< Threshold, Peak amplitude of range [0-2047]
} FilterNoiseGateData_t;


// Structure used to hold compressor (and expander) data
typedef struct
{
	uint16_t sensitivity;	///< number of samples over which to take an average
	uint16_t threshold;		///< Threshold, Peak amplitude of range [0-2047]
	float scalar;			///< float [0-1] scalar value to compress/expand by
} FilterCompressorData_t;


int16_t filter_noisegate_apply(int16_t input, void *pUnknown);
void filter_noisegate_debug(void *pUnknown);
void filter_noisegate_create(void *pUnknown);
int16_t filter_compressor_apply(int16_t input, void *pUnknown);
void filter_compressor_debug(void *pUnknown);
void filter_compressor_create(void *pUnknown);
int16_t filter_expander_apply(int16_t input, void *pUnknown);
void filter_expander_create(void *pUnknown);

#endif
