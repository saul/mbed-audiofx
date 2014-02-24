#include <stdint.h>
#include <stdlib.h>
#include <math.h>

#include "dbg.h"
#include "samples.h"
#include "fir.h"
#include "config.h"


#define FREQ_TO_PI_FRAC(hz) (2 * hz / ((float)SAMPLE_RATE))


uint32_t filter_fir_apply(uint32_t input, void *pPrivate)
{
	const FilterFIRBaseData_t *pData = (const FilterFIRBaseData_t *)pPrivate;

	uint32_t output = 0;

	for(uint8_t i = 0; i < pData->nCoefficients; ++i)
	{
		uint32_t iSample = i == 0 ? input : sample_get(-i);
		output += iSample * pData->pflCoefficients[i];
	}

	return output;
}


void filter_bandpass_debug(void *pPrivate)
{
	const FilterBandPassData_t *pData = (const FilterBandPassData_t *)pPrivate;
	dbg_printf("base.coeffs=%p, base.coeffs=%u, lower=%u, upper=%u", (void *)pData->base.pflCoefficients, pData->base.nCoefficients, pData->iLowerFreq, pData->iUpperFreq);
}


void filter_bandpass_mod(void *pPrivate)
{
	FilterBandPassData_t *pData = (FilterBandPassData_t *)pPrivate;

	// Free current coefficients and allocate space for the new ones
	free(pData->base.pflCoefficients);
	pData->base.pflCoefficients = malloc(sizeof(float) * pData->base.nCoefficients);
	dbg_assert(pData->base.pflCoefficients, "unable to allocate memory for %d FIR coefficients", pData->base.nCoefficients);

	float d1 = (pData->base.nCoefficients - 1) / 2.0f;
	float fc1 = FREQ_TO_PI_FRAC(pData->iLowerFreq);
	float fc2 = FREQ_TO_PI_FRAC(pData->iUpperFreq);

	dbg_printf("filter_bandpass_mod: coeffs=%d, lowcut=%.3f, uppercut=%.3f\r\n", pData->base.nCoefficients, fc1, fc2);

	// Calculate new coefficients
	for(uint8_t i = 0; i < pData->base.nCoefficients; ++i)
	{
		float d2 = i - d1;
		float flCoeff;

		if(d2 == 0)
			flCoeff = (fc2 - fc1) / PI_F;
		else
			flCoeff = (sinf(fc2 * d2) - sinf(fc1 * d2)) / (PI_F * d2);

		//dbg_printf("\t#%d: %f\r\n", i, flCoeff);

		pData->base.pflCoefficients[i] = flCoeff;
	}

	//dbg_printf("\r\n");
}
