/*
 * sys_timers.c
 *
 *  Created on: Feb 28, 2021
 *      Author: damonaknuh
 */
#include "sys_project.h"
#include "sys_tasks.h"
#include "sys_timer.h"


xTimerHandle xTIM_ADC_Sampler;
xTimerHandle xTIM_Light_Timer;

void TIM_Traffic_Light_cb(xTimerHandle xTimer)
{
	uint8_t nextState = g_CurrentLState << 1;

	if (nextState == MAX_STATE)
	{
		nextState = GREEN_STATE;
	}

	xEventGroupSetBits(xEVT_FSM_Transition,	nextState);
}


void TIM_ADC_Sampler_cb(xTimerHandle xTimer)
{
	printf("ADC Value: %d\n", controlADC.flowRate);

	if(controlADC.adcStatus == ADC_CONVERSION_OFF )
	{
		controlADC.adcStatus = ADC_CONVERSION_ON;
		ADC_SoftwareStartConv(ADC1);
	}
}

void my_SW_TIM_Init()
{
	xTIM_ADC_Sampler = xTimerCreate("ADCSampler", 1000 / portTICK_PERIOD_MS, pdTRUE,  (void *) 0, TIM_ADC_Sampler_cb);
	xTIM_Light_Timer = xTimerCreate("ADCLightWD", 1000 / portTICK_PERIOD_MS, pdFALSE, (void *) 0, TIM_Traffic_Light_cb);

	xTimerStart(xTIM_ADC_Sampler, 0);
}

