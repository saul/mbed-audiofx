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
#include "packets.h"


ChainStageHeader_t *g_pChainRoot = NULL;


static void time_tick(void *pUserData)
{
	// Do we have a sample ready to read in?
	if(!ADC_ChannelGetStatus(LPC_ADC, ADC_CHANNEL_0, ADC_DATA_DONE))
		return;

	uint16_t iSample = ADC_ChannelGetData(LPC_ADC, ADC_CHANNEL_0);
	sample_set(g_iSampleCursor, iSample);

	uint16_t iFiltered = iSample >> 2;

	sample_clear_average();

	// If we have a filter chain, apply all filters to the sample
	if(g_pChainRoot)
		iFiltered = chain_apply(g_pChainRoot, iSample);

	// Output to DAC
	dac_set(iFiltered);

	// Increase sample cursor
	g_iSampleCursor = (g_iSampleCursor + 1) % BUFFER_SAMPLES;
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
	adc_config(0, ENABLE);
	adc_start(ADC_START_CONTINUOUS);
	adc_burst_config(ENABLE);

	// DAC init
	dac_init();

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
	g_pChainRoot = stage_alloc(1);

	// Stage 1
	//-------------------------------------------
	ChainStage_t *pStage1 = STAGE_BY_INDEX(g_pChainRoot, 0);
	pStage1->pFilter = &g_pFilters[FILTER_DELAY];
	pStage1->flags = STAGEFLAG_FULL_MIX;

	FilterDelayData_t delayData;
	delayData.nDelay = 5000; // ~0.5 secs
	pStage1->pPrivate = &delayData;

	// Stage 2
	//-------------------------------------------
	ChainStageHeader_t *pStage2 = stage_alloc(2);
	g_pChainRoot->pNext = pStage2;

	ChainStage_t *pStage2b1 = STAGE_BY_INDEX(pStage2, 0);
	pStage2b1->pFilter = &g_pFilters[FILTER_DELAY];

	FilterDelayData_t delayData2b1;
	delayData2b1.nDelay = 7500; // ~0.75 secs
	pStage2b1->pPrivate = &delayData2b1;
	pStage2b1->flMixPerc = 0.5;


	ChainStage_t *pStage2b2 = STAGE_BY_INDEX(pStage2, 1);
	pStage2b2->pFilter = &g_pFilters[FILTER_DELAY];

	FilterDelayData_t delayData2b2;
	delayData2b2.nDelay = 2500; // ~0.25 secs
	pStage2b2->pPrivate = &delayData2b2;
	pStage2b2->flMixPerc = 0.5;

	// Debug chain
	//-------------------------------------------
	chain_debug(g_pChainRoot);

	//-----------------------------------------------------
	// Start sampling interrupt microtimer
	//-----------------------------------------------------
	microtimer_enable(0, TIM_PRESCALE_USVAL, 100, 10000 / SAMPLE_RATE, time_tick, NULL);

	while(1)
	{
		packet_reset_wait();
	}
}
