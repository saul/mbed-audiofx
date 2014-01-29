#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-pedantic"
#	include "lpc17xx_systick.h"
#pragma GCC diagnostic pop

#include "dbg.h"

static int s_bTimeSetup = 0;
static volatile unsigned long s_ulTickCount;
static float s_flSecsPerTick;


void SysTick_Handler(void)
{
	s_ulTickCount++;
}


void time_init(int iResMsec)
{
	s_flSecsPerTick = iResMsec / 1000.0;
	s_ulTickCount = 0;

	SYSTICK_InternalInit(iResMsec);
	SYSTICK_Cmd(ENABLE);
	SYSTICK_IntCmd(ENABLE);

	s_bTimeSetup = 1;
}


int time_setup(void)
{
	return s_bTimeSetup;
}


float time_realtime(void)
{
	dbg_assert(s_bTimeSetup, "time not initialised");

	return s_ulTickCount * s_flSecsPerTick;
}


void time_sleep(int msec)
{
	dbg_assert(s_bTimeSetup, "time not initialised");

	unsigned long ulStartTicks = s_ulTickCount;
	unsigned long ulSleepTicks = msec / (s_flSecsPerTick * 1000);

	while((s_ulTickCount - ulStartTicks) < ulSleepTicks);
}


unsigned long time_tickcount(void)
{
	dbg_assert(s_bTimeSetup, "time not initialised");
	return s_ulTickCount;
}


float time_tickinterval(void)
{
	dbg_assert(s_bTimeSetup, "time not initialised");
	return s_flSecsPerTick;
}
