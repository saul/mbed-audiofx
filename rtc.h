/*
 * rtc.c - Real-Time Clock functions
 *
 * Defines several functions for initialising the RTC peripheral.
 */

#ifndef _RTC_H_
#define _RTC_H_

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-pedantic"
#	include "lpc17xx_rtc.h"
#pragma GCC diagnostic pop

void rtc_init(void);

#endif
