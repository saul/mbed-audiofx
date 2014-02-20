#ifndef _DAC_H_
#define _DAC_H_

#define DAC_DEV LPC_DAC

void dac_init(void);
void dac_bias(uint32_t bias);
void dac_set(uint16_t val);

#endif
