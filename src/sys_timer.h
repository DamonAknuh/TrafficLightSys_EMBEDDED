/*
 * sys_timer.h
 *
 *  Created on: Feb 28, 2021
 *      Author: damonaknuh
 */

#ifndef SYS_TIMER_H_
#define SYS_TIMER_H_

#include "sys_project.h"

#define RED_LED_0_MS_0 		(2048)
#define GREEN_LED_0_MS 		(4096)
#define YELLOW_L_MS 		(1000)
#define MS_CHANGE_PER_ADC 	(2048 / MAX_ADC_VALUE)

extern xTimerHandle xTIM_Light_Timer;
extern xTimerHandle xTIM_ADC_Sampler;

void TIM_ADC_Sampler_cb(xTimerHandle xTimer);
void my_SW_TIM_Init();

#endif /* SYS_TIMER_H_ */
