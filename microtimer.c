#include "dbg.h"
#include "microtimer.h"

typedef struct
{
	TimerHandler_t pfnHandler;
	void *pUserData;
} Timer_t;

static Timer_t s_pHandlers[UTIM_NUM_TIMERS];


static void microtimer_irq_handler(uint8_t channel)
{
	if(TIM_GetIntStatus(LPC_TIM0 + channel, TIM_MR0_INT) == SET)
	{
		const Timer_t *pTimer = &s_pHandlers[channel];
		//dbg_assert(pTimer->pfnHandler != NULL, "timer interrupt fired on NULL handler (channel %u)", channel);
		pTimer->pfnHandler(pTimer->pUserData);
	}

	TIM_ClearIntPending(LPC_TIM0 + channel, TIM_MR0_INT);
}


void TIMER0_IRQHandler(void) { microtimer_irq_handler(0); }
void TIMER1_IRQHandler(void) { microtimer_irq_handler(1); }
void TIMER2_IRQHandler(void) { microtimer_irq_handler(2); }
void TIMER3_IRQHandler(void) { microtimer_irq_handler(3); }


void microtimer_enable(uint8_t channel, uint8_t prescaleOption, uint32_t prescaleVal, uint32_t matchValue, TimerHandler_t pfnHandler, void *pUserData)
{
	dbg_assert(pfnHandler != NULL, "handler must not be NULL");
	dbg_assert(channel < UTIM_NUM_TIMERS, "invalid channel (%u, max=%d)", channel, UTIM_NUM_TIMERS-1);

	Timer_t *pTimer = &s_pHandlers[channel];
	pTimer->pfnHandler = pfnHandler;
	pTimer->pUserData = pUserData;

	TIM_TIMERCFG_Type timerConfig;
	timerConfig.PrescaleOption = prescaleOption;
	timerConfig.PrescaleValue = prescaleVal;

	TIM_MATCHCFG_Type matchConfig;
	matchConfig.MatchChannel = channel;
	matchConfig.IntOnMatch = TRUE;
	matchConfig.ResetOnMatch = TRUE;
	matchConfig.StopOnMatch = FALSE;
	matchConfig.ExtMatchOutputType = TIM_EXTMATCH_NOTHING;
	matchConfig.MatchValue = matchValue;

	TIM_Init(LPC_TIM0 + channel, TIM_TIMER_MODE, &timerConfig);
	TIM_ConfigMatch(LPC_TIM0 + channel, &matchConfig);

	NVIC_SetPriority(TIMER0_IRQn + channel, (0x01 << 3) | 0x01); // preemption = 1, subpriority = 1
	NVIC_EnableIRQ(TIMER0_IRQn + channel);
	TIM_Cmd(LPC_TIM0 + channel, ENABLE);
}


void microtimer_disable(uint8_t channel)
{
	dbg_assert(channel < UTIM_NUM_TIMERS, "invalid channel (%u, max=%d)", channel, UTIM_NUM_TIMERS-1);

	TIM_Cmd(LPC_TIM0 + channel, DISABLE);
	NVIC_DisableIRQ(TIMER0_IRQn + channel);
}
