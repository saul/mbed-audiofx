#ifndef _MICROTIMER_H_
#define _MICROTIMER_H_

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-pedantic"
#	include "lpc17xx_timer.h"
#pragma GCC diagnostic pop

#define UTIM_NUM_TIMERS 4

typedef void (*TimerHandler_t)(void *pUserData);

void microtimer_enable(uint8_t channel, uint8_t prescaleOption, uint32_t prescaleVal, uint32_t matchValue, TimerHandler_t pfnHandler, void *pUserData);
void microtimer_disable(uint8_t channel);

#endif
