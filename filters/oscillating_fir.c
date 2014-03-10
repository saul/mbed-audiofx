#include <stdint.h>
#include <stdlib.h>
#include <math.h>

#include "dbg.h"
#include "samples.h"
#include "fir.h"
#include "config.h"
#include "waves.h"
#include "oscillating_fir.h"


int32_t filter_oscillating_bandpass_apply(int32_t input, void *pUnknown)
{
	FilterOscillatingBandPassData_t *pData = (FilterOscillatingBandPassData_t *)pUnknown;

	switch (pData->waveType)
	{
		case 0:
			if(get_square(pData->flOscillatingFrequency))
				pData->bandPass.iCentreFreq = pData->iMinFreq;
			else
				pData->bandPass.iCentreFreq = pData->iMaxFreq;
		case 1:
			pData->bandPass.iCentreFreq = pData->iMinFreq + ((pData->iMaxFreq - pData->iMinFreq) * get_sawtooth(pData->flOscillatingFrequency));
		case 2:
			pData->bandPass.iCentreFreq = pData->iMinFreq + ((pData->iMaxFreq - pData->iMinFreq) * get_inverse_sawtooth(pData->flOscillatingFrequency));
		case 3:
			pData->bandPass.iCentreFreq = pData->iMinFreq + ((pData->iMaxFreq - pData->iMinFreq) * get_triangle(pData->flOscillatingFrequency));
	}

	filter_bandpass_mod(pUnknown);
	return filter_fir_apply(input, pUnknown);
}


void filter_oscillating_bandpass_debug(void *pUnknown)
{
	const FilterOscillatingBandPassData_t *pData = (const FilterOscillatingBandPassData_t *)pUnknown;
	dbg_printf("frequency=%f, waveType=%u, minFreq=%u, maxFreq=%u, bandpass -> ",
					pData->flOscillatingFrequency, pData->waveType, pData->iMinFreq, pData->iMaxFreq);
	filter_bandpass_debug(pUnknown);
}


void filter_oscillating_bandpass_create(void *pUnknown)
{
	FilterOscillatingBandPassData_t *pData = (FilterOscillatingBandPassData_t *)pUnknown;
	pData->flOscillatingFrequency = 1;
	pData->waveType = 0;
	pData->iMinFreq = 50;
	pData->iMaxFreq = 1000;
	filter_bandpass_create(pUnknown);
}
