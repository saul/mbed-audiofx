#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>
#include <math.h>
#include <string.h>

// common
#include "sercom.h"
#include "ticktime.h"
#include "led.h"
#include "dbg.h"
#include "i2c.h"
#include "lcd.h"
#include "adc.h"
#include "dac.h"
#include "microtimer.h"

// audiofx
#include "config.h"
#include "chain.h"
#include "samples.h"
#include "filters.h"
#include "filters/delay.h"
#include "filters/dynamic.h"
#include "filters/vibrato.h"
#include "packets.h"


ChainStageHeader_t *g_pChainRoot = NULL;
volatile bool g_bChainLock = false;
volatile bool g_bPassThru = false;
volatile float g_flChainVolume = 1.0;
volatile uint32_t g_ulLastLongTick = 0;

#ifdef INDIVIDUAL_BUILD_TOM
volatile uint32_t iAnalogAverage = 0;
volatile bool bDoSendAverage = false;
volatile uint16_t iNumMeasurements = 0;
#endif // INDIVIDUAL_BUILD_TOM


static uint16_t get_median_sample(void)
{
	uint16_t iSamples[] = {
		ADC_ChannelGetData(LPC_ADC, ADC_CHANNEL_0),
		ADC_ChannelGetData(LPC_ADC, ADC_CHANNEL_4),
		ADC_ChannelGetData(LPC_ADC, ADC_CHANNEL_5),
	};

	if(iSamples[0] > iSamples[1])
	{
		if(iSamples[1] > iSamples[2])
			return iSamples[1];

		if(iSamples[0] > iSamples[2])
			return iSamples[2];

		return iSamples[0];
	}

	if(iSamples[0] > iSamples[2])
		return iSamples[0];

	if(iSamples[1] > iSamples[2])
		return iSamples[2];

	return iSamples[1];
}


static void time_tick(void *pUserData)
{
	static uint32_t s_ulLastClipTick = 0;

	uint32_t ulStartTick = time_tickcount();

	// Grab median sample from 3 ADC inputs (removes most of salt+pepper noise)
	uint16_t iSample = get_median_sample();
	sample_set(g_iSampleCursor, iSample);

	// Scale sample down to 10-bit (DAC resolution)
	uint16_t iFiltered = iSample >> 2;

	// If the filter chain is locked for modification (e.g., by a packet
	// handler), don't try to apply the chain. It may be in an intermediate
	// state/cause crashes/sound funky. Just passthru.
	if(!g_bChainLock && !g_bPassThru)
	{
		led_set(LED_PASS_THRU, false);
		sample_clear_average();

		// If we have a filter chain, apply all filters to the sample
		if(g_pChainRoot)
			iFiltered = chain_apply(g_pChainRoot, iSample);
	}
	else
		led_set(LED_PASS_THRU, true);

	// Output to DAC
	uint16_t iScaledOut = iFiltered * g_flChainVolume;
	dac_set(iScaledOut);

	// Increase sample cursor
	g_iSampleCursor = (g_iSampleCursor + 1) % BUFFER_SAMPLES;
	g_iWaveCursor = (g_iWaveCursor + 1) % (BUFFER_SAMPLES * 4);

#ifdef VIBRATO_TO_FIX
	if(g_bVibratoActive)
		g_iVibratoSampleCursor = vibrato_get_cursor();
#endif

	uint32_t ulEndTick = time_tickcount();

	// Clipped?
	if(iScaledOut == DAC_MAX_VALUE)
	{
		s_ulLastClipTick = ulEndTick;
		led_set(LED_CLIP, true);
	}

	// If we haven't clipped in 100 ticks, turn off the clip LED
	else if(s_ulLastClipTick + 100 < ulEndTick)
		led_set(LED_CLIP, false);

	// If we took longer than a millisecond to process sample, error
	// Assumes resolution is 1 tick/msec
	uint32_t ulElapsedTicks = ulEndTick - ulStartTick;

	if(ulElapsedTicks >= 1)
	{
		if(g_ulLastLongTick + 1000 < ulEndTick)
			dbg_printf(ANSI_COLOR_RED "Chain too complex" ANSI_COLOR_RESET ": sample took %lu msec to process!\r\n", ulElapsedTicks);

		g_ulLastLongTick = ulEndTick;
		led_set(LED_SLOW, true);
	}

	// If we haven't had been slow in 100 ticks, turn off the slow LED
	else if(g_ulLastLongTick + 100 < ulEndTick)
		led_set(LED_SLOW, false);
}


void main(void)
{
	//-----------------------------------------------------
	// Initialisation
	//-----------------------------------------------------
	// Show all LEDs on init
	led_init();
	led_set(0, true);
	led_set(1, true);
	led_set(2, true);
	led_set(3, true);

	// Initialise serial communication
	// Waits for UI to start before continuing boot sequence
	sercom_init();

	// Initialise timer
	time_init(1); // resolution: 1ms
	uint32_t ulStartTick = time_tickcount();

	// I2C init
	i2c_init();
	//i2c_scan();

	// LCD init
	//lcd_init();

	// ADC init
	adc_init(SAMPLE_RATE);
	adc_config(0, true); // MBED pin 15
	adc_config(4, true); // MBED pin 19
	adc_config(5, true); // MBED pin 20
#ifdef INDIVIDUAL_BUILD_TOM
	adc_config(1, true); // MBED pin 16
#endif // INDIVIDUAL_BUILD_TOM
	adc_start(ADC_START_CONTINUOUS);
	adc_burst_config(true);

	// DAC init
	dac_init();

	// Check static assertions (does nothing at run-time)
	packet_static_assertions();

	// Clear sample buffer
	for(uint16_t i = 0; i < BUFFER_SAMPLES; ++i)
		sample_set(i, 0);

	// Debug filters
	filter_debug();

	// Send filter list to UI (finalises boot sequence)
	packet_filter_list_send();

	// Generate an empty filter chain
	g_pChainRoot = stage_alloc();

	// Startup complete
	// Assumes resolution is 1 tick/msec
	uint32_t ulElapsedTicks = time_tickcount() - ulStartTick;
	dbg_printf(ANSI_COLOR_GREEN "Startup took %lu msec\r\n\n" ANSI_COLOR_RESET, ulElapsedTicks);
	led_blink(100, 5);

	//-----------------------------------------------------
	// Start sampling interrupt microtimer
	//-----------------------------------------------------
	microtimer_enable(0, TIM_PRESCALE_USVAL, 100, 10000 / SAMPLE_RATE, time_tick, NULL);

	//-----------------------------------------------------
	// Packet receive loop
	//-----------------------------------------------------
	while(1)
	{
		packet_loop();
#ifdef INDIVIDUAL_BUILD_TOM
		if(iNumMeasurements == 500)
		{
			packet_analog_control_send(iAnalogAverage/iNumMeasurements);
			iAnalogAverage = 0;
			iNumMeasurements = 0;
		}
		else
		{
			iAnalogAverage += ADC_ChannelGetData(LPC_ADC, ADC_CHANNEL_1);
			iNumMeasurements++;
		}
#endif
	}
}
