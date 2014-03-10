#ifndef _FILTER_DELAY_H_
#define _FILTER_DELAY_H_

typedef struct
{
	uint16_t nDelay; ///< sample delay
	float flDelayMixPerc;
} FilterDelayData_t;


uint32_t filter_delay_apply(uint32_t input, void *pUnknown);
void filter_delay_debug(void *pUnknown);
void filter_delay_create(void *pUnknown);
uint32_t filter_delay_feedback_apply(uint32_t input, void *pUnknown);

#endif
