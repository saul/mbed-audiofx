/*
 *	HAPR Project 2014
 *	Group 6 - Tom Bryant (TB) & Saul Rennison (SR)
 *
 *	File modified from TB & SR Mini-Project work
 *
 * ticktime.c - Tick timer functions
 *
 * Defines functions for initialising and obtaining tick timer values
 */


#ifndef _TICKTIME_H_
#define _TICKTIME_H_

#include <stdbool.h>

void time_init(uint32_t iResMsec);
bool time_setup(void);
float time_realtime(void);
void time_sleep(uint32_t msec);
uint32_t time_tickcount(void);
float time_tickinterval(void);

#endif
