/*
 *	HAPR Project 2014
 *	Group 6 - Tom Bryant (TB) & Saul Rennison (SR)
 *	Saul Rennison Individual Part
 *
 *	File created by:	SR
 *	File modified by:	SR
 *	File debugged by:	SR
 *
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
