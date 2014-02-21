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
const uint8_t NUM_LEDS = 4;

static bool s_pbLedStates[] = {false, false, false, false};
static bool s_bLEDSetup = false;


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


bool led_setup(void)
{
	return s_bLEDSetup;
}


void led_set(uint8_t led, bool bState)
{
	dbg_assert(s_bLEDSetup, "LEDs not initialised");
	dbg_assert(led < NUM_LEDS, "invalid LED index");

	if(s_pbLedStates[led] == bState)
		return;

	if(bState)
		GPIO_SetValue(1, LED_MASKS[led]);
	else
		GPIO_ClearValue(1, LED_MASKS[led]);

	s_pbLedStates[led] = bState;
}


void led_flip(uint8_t led)
{
	dbg_assert(led < NUM_LEDS, "invalid LED index");
	led_set(led, !s_pbLedStates[led]);
}


bool led_get(uint8_t led)
{
	dbg_assert(s_bLEDSetup, "LEDs not initialised");
	dbg_assert(led < NUM_LEDS, "invalid LED index");
	return s_pbLedStates[led];
}


void led_clear(void)
{
	dbg_assert(s_bLEDSetup, "LEDs not initialised");

	memset(s_pbLedStates, sizeof(s_pbLedStates), 0);
	GPIO_ClearValue(1, ALL_LEDS);
}


bool led_show_bin(uint8_t value)
{
	led_clear();

	if(value >= (1 << NUM_LEDS))
		return false;

	for(uint8_t i = 0; i < NUM_LEDS; ++i)
		led_set(i, value & (1 << i));

	return true;
}


void led_blink(uint32_t msec_interval, uint16_t count)
{
	dbg_assert(s_bLEDSetup, "LEDs not initialised");
	dbg_assert(time_setup(), "time not initialised");

	for(uint16_t i = 0; !count || i < count; ++i)
	{
		GPIO_SetValue(1, ALL_LEDS);
		time_sleep(msec_interval/2);
		GPIO_ClearValue(1, ALL_LEDS);
		time_sleep(msec_interval/2);
	}

	led_clear();
}
