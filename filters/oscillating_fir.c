#include <stdint.h>
#include <stdlib.h>
#include <math.h>

#include "dbg.h"
#include "samples.h"
#include "fir.h"
#include "config.h"
#include "waves.h"
#include "oscillating_fir.h"


uint32_t filter_oscillating_bandpass_apply(uint32_t input, void *pUnknown)
{
	const FilterOscillatingBandPassData_t *pData = (const FilterOscillatingBandPassData_t *)pUnknown;

	switch (pData->waveType)
	{
		case 0:
			if(get_square(pData->flOscillatingFrequency))
				pData->bandPass.iCentreFreq = pData->iMinFreq;
			else
				pData->bandPass.iCentreFreq = pData->iMaxFreq;
		case 1:
			pData->bandPass.iCentreFreq = pData->iMinFreq + ((pData->iMaxFreq - pData->iMinFreq) * get_sawtooth(pData->frequency));
		case 2:
			pData->bandPass.iCentreFreq = pData->iMinFreq + ((pData->iMaxFreq - pData->iMinFreq) * get_inverse_sawtooth(pData->frequency));
		case 3:
			pData->bandPass.iCentreFreq = pData->iMinFreq + ((pData->iMaxFreq - pData->iMinFreq) * get_triangle(pData->frequency));
	}

	return filter_fir_apply(input, pData->bandPass);
}


void filter_oscillating_bandpass_mod(void *pUnknown)
{
	FilterOscillatingBandPassData_t *pData = (FilterOscillatingBandPassData_t *)pUnknown;

	pData->bandPass.base.nCoefficients
}