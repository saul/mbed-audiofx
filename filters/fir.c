/*
 *	HAPR Project 2014
 *	Group 6 - Tom Bryant (TB) & Saul Rennison (SR)
 *
 *	File created by:	SR
 *	File modified by:	SR
 *	File debugged by:	SR
*/


#include <stdint.h>
#include <stdlib.h>
#include <math.h>

#include "dbg.h"
#include "samples.h"
#include "fir.h"
#include "config.h"


#define FREQ_TO_PI_FRAC(hz) (2 * hz / ((float)SAMPLE_RATE))


/*
 *	FIR apply uses the calculated coefficients to sum together
 *	the previous 'pData->nCoefficients' samples, each independently
 *	scaled by their corresponding coefficient.
 *	This creates a bandpass effect.
 *
 *	inputs:
 *		input		signed 12 bit sample
 *		pUnknown	null pointer to FilterFIRBaseData_t data
 *
 *	output:
 *		signed 12 bit sample
 */
int16_t filter_fir_apply(int16_t input, void *pUnknown)
{
	const FilterFIRBaseData_t *pData = (const FilterFIRBaseData_t *)pUnknown;

	int16_t output = 0;

	for(uint8_t i = 0; i < pData->nCoefficients; ++i)
	{
		int16_t iSample = i == 0 ? input : sample_get(-i);
		output += iSample * pData->pflCoefficients[i];
	}

	return output;
}


// Print bandpass parameter values to UI console
void filter_bandpass_debug(void *pUnknown)
{
	const FilterBandPassData_t *pData = (const FilterBandPassData_t *)pUnknown;
	dbg_printf("base.coeffs=%p, base.coeffs=%u, centre=%u, width=%u", (void *)pData->base.pflCoefficients, pData->base.nCoefficients, pData->iCentreFreq, pData->iWidth);
}


/*
 *	As recalculation of coefficients takes a long time,
 *	filter_bandpass_mod function is used to allow the
 *	coefficients to be calculated only once per parameter
 *	change.
 *	First, it is ensured that their is enough space to store
 *	the parameters by allocating them in memory before
 *	calculating.
 *	Then, each coefficient is calculated and stored in the
 *	array.
 */
void filter_bandpass_mod(void *pUnknown)
{
	FilterBandPassData_t *pData = (FilterBandPassData_t *)pUnknown;

	dbg_assert(pData->base.nCoefficients > 0, "coefficient number must be > 0");

	// Free current coefficients and allocate space for the new ones
	free(pData->base.pflCoefficients);
	pData->base.pflCoefficients = malloc(sizeof(float) * pData->base.nCoefficients);
	dbg_assert(pData->base.pflCoefficients, "unable to allocate memory for %d FIR coefficients", pData->base.nCoefficients);

	int32_t iLowerFreq = fmaxf(pData->iCentreFreq - (pData->iWidth / 2), 0);
	int32_t iUpperFreq = fminf(pData->iCentreFreq + (pData->iWidth / 2), 20000);

	float d1 = (pData->base.nCoefficients - 1) / 2.0f;
	float fc1 = FREQ_TO_PI_FRAC(iLowerFreq);
	float fc2 = FREQ_TO_PI_FRAC(iUpperFreq);

	// Calculate new coefficients
	for(uint8_t i = 0; i < pData->base.nCoefficients; ++i)
	{
		float d2 = i - d1;
		float flCoeff;

		if(d2 == 0)
			flCoeff = (fc2 - fc1) / PI_F;
		else
			flCoeff = (sinf(fc2 * d2) - sinf(fc1 * d2)) / (PI_F * d2);

		pData->base.pflCoefficients[i] = flCoeff;
	}
}


// Set initial bandpass creation parameters
void filter_bandpass_create(void *pUnknown)
{
	FilterBandPassData_t *pData = (FilterBandPassData_t *)pUnknown;
	pData->base.nCoefficients = 15;
	pData->iCentreFreq = 1000;
	pData->iWidth = 500;

	// Generate coefficients
	filter_bandpass_mod(pUnknown);
}
