#ifndef _CONFIG_H_
#define _CONFIG_H_

#define PI_F			3.14159265359f

#define SAMPLE_RATE		10000
#define BUFFER_SAMPLES	10000
#define ADC_MAX_VALUE	((1<<12)-1)
#define DAC_MAX_VALUE	((1<<10)-1)
#define ADC_MID_POINT	((ADC_MAX_VALUE+1)/2)

#define LED_CLIP		0
#define LED_SLOW		1
#define LED_PASS_THRU	3

#endif
