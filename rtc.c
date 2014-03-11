/*
 * rtc.c - Real-Time Clock functions
 *
 * Defines several functions for initialising the RTC peripheral.
 */

#include "rtc.h"


void rtc_init(void)
{
	// Init RTC module
	RTC_Init(LPC_RTC);

	// Enable RTC (starts increase the tick counter and second counter register)
	RTC_ResetClockTickCounter(LPC_RTC);
	RTC_Cmd(LPC_RTC, ENABLE);

	// Set fake time to 1st April 2014
	RTC_TIME_Type rtcTime;
	rtcTime.SEC = 0;
	rtcTime.MIN = 0;
	rtcTime.HOUR = 0;
	rtcTime.DOM = 1;
	rtcTime.DOW = 2;
	rtcTime.DOY = 0;
	rtcTime.MONTH = 3;
	rtcTime.YEAR = 2014;
	RTC_SetFullTime(LPC_RTC, &rtcTime);
}
