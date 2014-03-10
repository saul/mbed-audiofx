#include <stdint.h>
#include <stdlib.h>
#include <math.h>

#include "dbg.h"
#include "samples.h"
#include "fir.h"
#include "config.h"


#define FREQ_TO_PI_FRAC(hz) (2 * hz / ((float)SAMPLE_RATE))


int32_t filter_fir_apply(int32_t input, void *pUnknown)
{
	const FilterFIRBaseData_t *pData = (const FilterFIRBaseData_t *)pUnknown;

	int32_t output = 0;

	for(uint8_t i = 0; i < pData->nCoefficients; ++i)
	{
		int32_t iSample = i == 0 ? input : sample_get(-i);
		output += iSample * pData->pflCoefficients[i];
	}

	return output;
}


void filter_bandpass_debug(void *pUnknown)
{
	const FilterBandPassData_t *pData = (const FilterBandPassData_t *)pUnknown;
	dbg_printf("base.coeffs=%p, base.coeffs=%u, centre=%u, width=%u", (void *)pData->base.pflCoefficients, pData->base.nCoefficients, pData->iCentreFreq, pData->iWidth);
}


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

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdouble-promotion"
	//dbg_printf("filter_bandpass_mod: coeffs=%d, lowcut=%.3f, uppercut=%.3f\r\n", pData->base.nCoefficients, fc1, fc2);
#pragma GCC diagnostic pop

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


void filter_bandpass_create(void *pUnknown)
{
	FilterBandPassData_t *pData = (FilterBandPassData_t *)pUnknown;
	pData->base.nCoefficients = 25;
	pData->iCentreFreq = 1000;
	pData->iWidth = 500;

	// Generate coefficients
	filter_bandpass_mod(pUnknown);
}
