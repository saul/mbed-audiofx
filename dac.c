#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-pedantic"
#	include "lpc17xx_dac.h"
#	include "lpc17xx_pinsel.h"
#pragma GCC diagnostic pop

#include "dac.h"

void dac_init(void)
{
	// MBED Pin 18
	PINSEL_CFG_Type pcfg;
	pcfg.Pinmode = 0;
	pcfg.Funcnum = 2;
	pcfg.Pinnum = 26;
	pcfg.Portnum = 0;
	PINSEL_ConfigPin(&pcfg);
	DAC_Init(DAC_DEV);
}


void dac_bias(uint32_t bias)
{
	DAC_SetBias(DAC_DEV, bias);
}


void dac_set(uint16_t val)
{
	DAC_UpdateValue(DAC_DEV, val);
}
