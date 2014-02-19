#include <stdlib.h>
#include <stdint.h>
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
#include "packets.h"
#include "vibrato.h"


ChainStageHeader_t *g_pChainRoot = NULL;
volatile bool g_bChainLock = false;

volatile bool g_bVibratoActive = false;


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
	// UNDONE: this doesn't seem to affect anything
#if 0
	// Do we have a sample ready to read in?
	if(!ADC_ChannelGetStatus(LPC_ADC, ADC_CHANNEL_0, ADC_DATA_DONE) ||
		!ADC_ChannelGetStatus(LPC_ADC, ADC_CHANNEL_4, ADC_DATA_DONE) ||
		!ADC_ChannelGetStatus(LPC_ADC, ADC_CHANNEL_5, ADC_DATA_DONE))
		return;
#endif

	uint16_t iSample = get_median_sample();
	sample_set(g_iSampleCursor, iSample);

	uint16_t iFiltered = iSample >> 2;

	// If the filter chain is locked for modification (e.g., by a packet
	// handler), don't try to apply the chain. It may be in an intermediate
	// state/cause crashes/sound funky. Just passthru.
	if(!g_bChainLock)
	{
		sample_clear_average();

		// If we have a filter chain, apply all filters to the sample
		if(g_pChainRoot)
			iFiltered = chain_apply(g_pChainRoot, iSample);
	}

	// Output to DAC
	dac_set(iFiltered);

	// Increase sample cursor
	g_iSampleCursor = (g_iSampleCursor + 1) % BUFFER_SAMPLES;

	if(g_bVibratoActive)
		g_fVibratoSampleCursor = get_vibrato_pointer();
}


void main(void)
{
	//-----------------------------------------------------
	// Initialisation
	//-----------------------------------------------------
	// Show all LEDs on init
	led_init();
	led_set(0, 1);
	led_set(1, 1);
	led_set(2, 1);
	led_set(3, 1);

	// Initialise serial console
	sercom_init();

	// Initialise timer
	time_init(1);
	unsigned long t = time_tickcount();

	// I2C init
	//i2c_init();
	//i2c_scan();

	// LCD init
	//lcd_init();

	// ADC init
	adc_init(SAMPLE_RATE);
	adc_config(0, ENABLE); // MBED pin 15
	adc_config(4, ENABLE); // MBED pin 19
	adc_config(5, ENABLE); // MBED pin 20
	adc_start(ADC_START_CONTINUOUS);
	adc_burst_config(ENABLE);

	// DAC init
	dac_init();

	// Check static assertions (does nothing at run-time)
	packet_static_assertions();

	// Startup complete
	t = time_tickcount() - t;
	dbg_printf(ANSI_COLOR_GREEN "Startup took %d msec\r\n\n" ANSI_COLOR_RESET, (int)(t * time_tickinterval() * 1000));
	led_blink(100, 5);

	// Clear sample buffer
	for(int i = 0; i < BUFFER_SAMPLES; ++i)
		sample_set(i, 0);

	// Debug filters
	filter_debug();
	packet_filter_list_send();

	//-----------------------------------------------------
	// Generate a filter chain
	//-----------------------------------------------------
	g_pChainRoot = stage_alloc();

	// Stage 1
	//-------------------------------------------
	ChainStageHeader_t *pStage1 = g_pChainRoot;

	// Stage 1, Branch 1
	// -----------------
	FilterDelayData_t *pDelayData;
	StageBranch_t *pBranch1 = branch_alloc(FILTER_DELAY, STAGEFLAG_ENABLED, 0.75, (void**)&pDelayData);

	pDelayData->nDelay = 2500; // ~0.25 secs

	pStage1->nBranches++;
	pStage1->pFirst = pBranch1;

#define MULTI_BRANCH_TEST
#ifdef MULTI_BRANCH_TEST
	// Stage 1, Branch 2
	// -----------------
	FilterDelayData_t *pDelayData2;
	StageBranch_t *pBranch2 = branch_alloc(FILTER_DELAY, STAGEFLAG_ENABLED, 0.35, (void**)&pDelayData2);

	pDelayData2->nDelay = 7500; // ~0.75 secs

	pStage1->nBranches++;
	pBranch1->pNext = pBranch2;
#endif

	// Debug chain
	//-------------------------------------------
	chain_debug(g_pChainRoot);

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
	}
}
