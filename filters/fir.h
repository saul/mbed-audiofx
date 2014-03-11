/*
 *	HAPR Project 2014
 *	Group 6 - Tom Bryant (TB) & Saul Rennison (SR)
 *
 *	File created by:	SR
 *	File modified by:	SR
 *	File debugged by:	SR
 *
 *	fir.c
 *
 *	Defines functions to apply a band-pass filter to a sample.
*/


#ifndef _FILTER_FIR_H_
#define _FILTER_FIR_H_

#pragma pack(push, 1)
typedef struct
{
	float *pflCoefficients;	///< Pointer to the coefficients array
	uint8_t nCoefficients;	///< Number of coefficients to be calculated/used
} FilterFIRBaseData_t;
#pragma pack(pop)


#pragma pack(push, 1)
typedef struct
{
	FilterFIRBaseData_t base;
	uint16_t iCentreFreq;		///< Centre frequency of the band pass (Hz)
	uint16_t iWidth;			///< Width of the band pass (Hz)
} FilterBandPassData_t;
#pragma pack(pop)


int16_t filter_fir_apply(int16_t input, void *pUnknown);
void filter_bandpass_debug(void *pUnknown);
void filter_bandpass_mod(void *pUnknown);
void filter_bandpass_create(void *pUnknown);

#endif
