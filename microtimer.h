/*
 * microtimer.c - Match timer functions
 *
 * Defines several functions for managing microsecond timers.
 */

#ifndef _MICROTIMER_H_
#define _MICROTIMER_H_

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-pedantic"
#	include "lpc17xx_timer.h"
#pragma GCC diagnostic pop


// Number of match timers available
// `chan` in the functions below must be in the range [0..UTIM_NUM_TIMERS)
#define UTIM_NUM_TIMERS 4

// Prototype of timer callbacks (e.g., time_tick in main.c)
typedef void (*TimerHandler_t)(void *pUserData);


void microtimer_enable(uint8_t channel, uint8_t prescaleOption, uint32_t prescaleVal, uint32_t matchValue, TimerHandler_t pfnHandler, void *pUserData);
void microtimer_disable(uint8_t channel);

#endif
