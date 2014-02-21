#ifndef _ADC_H_
#define _ADC_H_

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-pedantic"
#	include "lpc17xx_adc.h"
#pragma GCC diagnostic pop

#define ADC_NUM_CHANNELS 6
#define ADC_DEV LPC_ADC

void adc_init(uint32_t rate);
void adc_config(uint8_t chan, bool bEnable);
void adc_interrupt_config(uint8_t chan, bool bEnable);
void adc_start(uint8_t mode);
uint16_t adc_read(uint8_t chan);
void adc_burst_config(bool bEnable);
FlagStatus adc_status(bool chan);

#endif
