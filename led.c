#include <string.h> // memset

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-pedantic"
#	include "lpc17xx_pinsel.h"
#	include "lpc17xx_gpio.h"
#pragma GCC diagnostic pop

#include "dbg.h"
#include "ticktime.h"
#include "led.h"


const unsigned long LED_MASKS[] = {1<<18, 1<<20, 1<<21, 1<<23};
const unsigned long ALL_LEDS = (1<<18) | (1<<20) | (1<<21) | (1<<23);
const int NUM_LEDS = 4;

static bool s_pbLedStates[] = {false, false, false, false};
static bool s_bLEDSetup = 0;


void led_init(void)
{
	PINSEL_CFG_Type PinCfg;

	PinCfg.Funcnum = 0;
	PinCfg.OpenDrain = 0;
	PinCfg.Pinmode = 0;

	PinCfg.Portnum = 1;
	PinCfg.Pinnum = 18;
	PINSEL_ConfigPin(&PinCfg);

	PinCfg.Portnum = 1;
	PinCfg.Pinnum = 20;
	PINSEL_ConfigPin(&PinCfg);

	PinCfg.Portnum = 1;
	PinCfg.Pinnum = 21;
	PINSEL_ConfigPin(&PinCfg);

	PinCfg.Portnum = 1;
	PinCfg.Pinnum = 23;
	PINSEL_ConfigPin(&PinCfg);

	GPIO_SetDir(1, ALL_LEDS, 1);
	s_bLEDSetup = true;

	led_clear();
}


int led_setup(void)
{
	return s_bLEDSetup;
}


int led_set(int led, bool bState)
{
	dbg_assert(s_bLEDSetup, "LEDs not initialised");

	if(led < 0 || led >= NUM_LEDS)
		return -1;

	if(s_pbLedStates[led] == bState)
		return 0;

	if(bState)
		GPIO_SetValue(1, LED_MASKS[led]);
	else
		GPIO_ClearValue(1, LED_MASKS[led]);

	s_pbLedStates[led] = bState;
	return 0;
}


int led_flip(int led)
{
	dbg_assert(s_bLEDSetup, "LEDs not initialised");

	if(led < 0 || led >= NUM_LEDS)
		return -1;

	return led_set(led, !s_pbLedStates[led]);
}


int led_get(int led)
{
	dbg_assert(s_bLEDSetup, "LEDs not initialised");

	if(led < 0 || led >= NUM_LEDS)
		return -1;

	return s_pbLedStates[led];
}


void led_clear(void)
{
	dbg_assert(s_bLEDSetup, "LEDs not initialised");

	memset(s_pbLedStates, sizeof(s_pbLedStates), 0);
	GPIO_ClearValue(1, ALL_LEDS);
}


int led_show_bin(int v)
{
	dbg_assert(s_bLEDSetup, "LEDs not initialised");

	led_clear();

	if(v < 0 || v >= (1 << NUM_LEDS))
		return -1;

	for(int i = 0; i < NUM_LEDS; ++i)
		led_set(i, v & (1 << i));

	return 0;
}


void led_blink(int msec_interval, int count)
{
	dbg_assert(s_bLEDSetup, "LEDs not initialised");
	dbg_assert(time_setup(), "time not initialised");

	for(int i = 0; count == -1 || i < count; ++i)
	{
		GPIO_SetValue(1, ALL_LEDS);
		time_sleep(msec_interval/2);
		GPIO_ClearValue(1, ALL_LEDS);
		time_sleep(msec_interval/2);
	}

	led_clear();
}

