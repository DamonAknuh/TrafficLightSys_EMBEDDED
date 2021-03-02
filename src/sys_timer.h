/*
 * sys_timer.h
 *
 *  Created on: Feb 28, 2021
 *      Author: damonaknuh
 */

#ifndef SYS_TIMER_H_
#define SYS_TIMER_H_

#include "sys_project.h"

#define RED_LED_0_MS_0 		(2048 / portTICK_PERIOD_MS)
#define GREEN_LED_0_MS 		(4096 / portTICK_PERIOD_MS)
#define YELLOW_L_MS 		(1000 / portTICK_PERIOD_MS)
#define MS_CHANGE_PER_ADC 	(4096 / MAX_ADC_VALUE)

extern xTimerHandle xTIM_Light_Timer;

void TIM_ADC_Sampler_cb(xTimerHandle xTimer);
void my_SW_TIM_Init();


#endif /* SYS_TIMER_H_ */
