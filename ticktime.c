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

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-pedantic"
#	include "lpc17xx_systick.h"
#pragma GCC diagnostic pop

#include <stdbool.h>
#include "dbg.h"

static bool s_bTimeSetup = false;
static volatile uint32_t s_ulTickCount = 0;
static float s_flSecsPerTick;


// Increment tickCount on interrupt
void SysTick_Handler(void)
{
	s_ulTickCount++;
}


// Initialise time to the resolution of 'iResMsec'
void time_init(uint32_t iResMsec)
{
	s_flSecsPerTick = iResMsec / 1000.0;

	SYSTICK_InternalInit(iResMsec);
	SYSTICK_Cmd(ENABLE);
	SYSTICK_IntCmd(ENABLE);

	s_bTimeSetup = true;
}


// Returns true if time has been initialised, else false
bool time_setup(void)
{
	return s_bTimeSetup;
}


float time_realtime(void)
{
	dbg_assert(s_bTimeSetup, "time not initialised");

	return s_ulTickCount * s_flSecsPerTick;
}


// Sleep function (sleeps for 'msec' milliseconds)
void time_sleep(uint32_t msec)
{
	dbg_assert(s_bTimeSetup, "time not initialised");

	uint32_t ulStartTicks = s_ulTickCount;
	uint32_t ulSleepTicks = msec / (s_flSecsPerTick * 1000);

	while((s_ulTickCount - ulStartTicks) < ulSleepTicks);
}


// Returns the current tick count
uint32_t time_tickcount(void)
{
	dbg_assert(s_bTimeSetup, "time not initialised");
	return s_ulTickCount;
}


// Returns the current tick interval
float time_tickinterval(void)
{
	dbg_assert(s_bTimeSetup, "time not initialised");
	return s_flSecsPerTick;
}
