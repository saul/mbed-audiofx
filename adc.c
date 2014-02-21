#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-pedantic"
#	include "lpc17xx_pinsel.h"
#pragma GCC diagnostic pop

#include "adc.h"
#include "dbg.h"

void adc_init(uint32_t rate)
{
	ADC_Init(ADC_DEV, rate);
}


// MBED Pin mapping: http://www-users.cs.york.ac.uk/~pcc/MCP/MbedPins.html
void adc_config(uint8_t chan, bool bEnable)
{
	dbg_assert(chan < ADC_NUM_CHANNELS, "invalid channel (%u, max=%u)", chan, ADC_NUM_CHANNELS-1);

	PINSEL_CFG_Type adc_pcfg;
	adc_pcfg.OpenDrain = 0;
	adc_pcfg.Pinmode = 0;
	adc_pcfg.Pinnum = 0;
	adc_pcfg.Portnum = 0;
	adc_pcfg.Funcnum = 1;

	switch(chan)
	{
	case 0: // MBED Pin 15
		adc_pcfg.Pinnum = 23;
		break;
	case 1: // MBED Pin 16
		adc_pcfg.Pinnum = 24;
		break;
	case 2: // MBED Pin 17
		adc_pcfg.Pinnum = 25;
		break;
	case 3: // MBED Pin 18 (also DAC@funcnum 2, mutually exclusive)
		adc_pcfg.Pinnum = 26;
		break;
	case 4: // MBED Pin 19 (audio socket on UoY host board)
		adc_pcfg.Pinnum = 30;
		adc_pcfg.Portnum = 1;
		adc_pcfg.Funcnum = 3;
		break;
	case 5: // MBED Pin 20
		adc_pcfg.Pinnum = 31;
		adc_pcfg.Portnum = 1;
		adc_pcfg.Funcnum = 3;
		break;
	default:
		// Unreachable
		return;
	}

	if(bEnable)
		PINSEL_ConfigPin(&adc_pcfg);

	ADC_ChannelCmd(ADC_DEV, chan, bEnable ? ENABLE : DISABLE);
}

void adc_interrupt_config(uint8_t chan, bool bEnable)
{
	dbg_assert(chan < ADC_NUM_CHANNELS, "invalid channel (%u, max=%u)", chan, ADC_NUM_CHANNELS-1);
	ADC_IntConfig(ADC_DEV, ADC_ADINTEN0 + chan, bEnable);
}

void adc_start(uint8_t mode)
{
	ADC_StartCmd(ADC_DEV, mode);
}

uint16_t adc_read(uint8_t chan)
{
	dbg_assert(chan < ADC_NUM_CHANNELS, "invalid channel (%u, max=%u)", chan, ADC_NUM_CHANNELS-1);

	uint16_t val = ADC_ChannelGetData(ADC_DEV, chan);

	// The ADC floats high. Return 0 if we're clipped
	if(val == (1<<12) - 1)
		val = 0;

	return val;
}

void adc_burst_config(bool bEnable)
{
	dbg_assert(bEnable <= 1, "invalid burst config (%u, expected boolean)", bEnable);
	ADC_BurstCmd(ADC_DEV, bEnable);
}

FlagStatus adc_status(uint8_t chan)
{
	dbg_assert(chan < ADC_NUM_CHANNELS, "invalid channel (%u, max=%u)", chan, ADC_NUM_CHANNELS-1);
	return ADC_ChannelGetStatus(ADC_DEV, chan, 1);
}
