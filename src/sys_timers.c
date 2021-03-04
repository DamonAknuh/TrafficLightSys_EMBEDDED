/*
 * sys_timers.c
 *
 *  Created on: Feb 28, 2021
 *      Author: damonaknuh
 */
#include "sys_project.h"
#include "sys_tasks.h"
#include "sys_timer.h"

void TIM_Traffic_Light_cb(xTimerHandle xTimer)
{
    uint8_t nextState;

    // ==> Retrieve the current state value
    xQueuePeek(xQ_LightState, &nextState, pdMS_TO_TICKS(1000));

    // ==> Increment the state.
    //          GREEN   --> YELLOW
    //          YELLOW  --> RED
    //          RED     --> GREEN
    nextState = nextState << 1;

    if (nextState == MAX_STATE)
    {
        nextState = GREEN_STATE;
    }

    xQueueOverwrite(xQ_LightState, &nextState);

    xEventGroupSetBits(xEVT_FSM_Transition,    nextState);
}


void TIM_ADC_Sampler_cb(xTimerHandle xTimer)
{
    uint8_t adc_value = 0;

    // ==> Ensure no conversion is ongoing.
    if(!ADC_GetSoftwareStartConvStatus(ADC1))
    {
        // ==> Retrieved value is between [0, 4096]
        adc_value = ADC_GetConversionValue(ADC1) / 32;

        xQueueOverwrite(xQ_FlowRate, &adc_value);

        ADC_SoftwareStartConv(ADC1);
    }
}

void my_SW_TIM_Init()
{
    const uint8_t flowRateInit = ADC_VALUE_INIT;

    // ==> Create flow rate queue for intertask communication
    xQ_FlowRate = xQueueCreate(QUEUE_LENGTH, QUEUE_ITEM_SIZE);
    xQueueSend(xQ_FlowRate, &flowRateInit, 0);

    xTIM_ADC_Sampler = xTimerCreate("ADCSampler", 1000 / portTICK_PERIOD_MS, pdTRUE,  (void *) 0, TIM_ADC_Sampler_cb);
    xTIM_Light_Timer = xTimerCreate("ADCLightWD", 1000 / portTICK_PERIOD_MS, pdFALSE, (void *) 0, TIM_Traffic_Light_cb);

    // == > start the ADC Sampler
    xTimerStart(xTIM_ADC_Sampler, 0);
}

