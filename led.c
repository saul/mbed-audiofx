/*
 * led.c - on-board LED functions
 *
 * Defines several functions for management of on-board LED state.
 */

#include <string.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-pedantic"
#	include "lpc17xx_pinsel.h"
#	include "lpc17xx_gpio.h"
#pragma GCC diagnostic pop

#include "dbg.h"
#include "ticktime.h"
#include "led.h"


// GPIO pin masks for each LED
const unsigned long LED_MASKS[] = {1<<18, 1<<20, 1<<21, 1<<23};

// OR of all GPIO pins
const unsigned long ALL_LEDS = (1<<18) | (1<<20) | (1<<21) | (1<<23);

// Number of LEDs available
const uint8_t NUM_LEDS = 4;

// Current state of each LED (on/off)
static bool s_pbLedStates[] = {false, false, false, false};

// Has led_init been called yet?
static bool s_bLEDSetup = false;


/*
 * led_init
 *
 * Initialises the LED pins and sets the correct GPIO direction.
 */
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


/*
 * led_setup
 *
 * Has `led_init` been called?
 */
bool led_setup(void)
{
	return s_bLEDSetup;
}


/*
 * led_set
 *
 * Sets an LED to on or off.
 */
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


/*
 * led_flip
 *
 * Flips the state of an LED.
 */
void led_flip(uint8_t led)
{
	dbg_assert(led < NUM_LEDS, "invalid LED index");
	led_set(led, !s_pbLedStates[led]);
}


/*
 * led_get
 *
 * Gets the current state of an LED.
 */
bool led_get(uint8_t led)
{
	dbg_assert(s_bLEDSetup, "LEDs not initialised");
	dbg_assert(led < NUM_LEDS, "invalid LED index");
	return s_pbLedStates[led];
}


/*
 * led_clear
 *
 * Turn off all LEDs.
 */
void led_clear(void)
{
	dbg_assert(s_bLEDSetup, "LEDs not initialised");

	memset(s_pbLedStates, sizeof(s_pbLedStates), 0);
	GPIO_ClearValue(1, ALL_LEDS);
}


/*
 * led_blink
 *
 * Blink all LEDs `count` times at an interval of `msec_interval`.
 * If count is LED_BLINK_INDEFINITE (0), blinks forever.
 *
 * Requires time subsystem to be initialised.
 */
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
