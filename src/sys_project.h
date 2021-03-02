/*
 * sys_project.h
 *
 */

#ifndef SYS_PROJECT_H
#define SYS_PROJECT_H

/* Standard includes. */
#include <stdint.h>
#include <stdio.h>
#include "stm32f4_discovery.h"

/* Kernel includes. */
#include "stm32f4xx.h"
#include "../FreeRTOS_Source/include/FreeRTOS.h"
#include "../FreeRTOS_Source/include/queue.h"
#include "../FreeRTOS_Source/include/semphr.h"
#include "../FreeRTOS_Source/include/task.h"
#include "../FreeRTOS_Source/include/timers.h"
#include "../FreeRTOS_Source/include/event_groups.h"

#define TRAFFIC_PORT 		GPIOC
#define GPIO_LED_RED  		GPIO_Pin_0
#define GPIO_LED_YELLOW 	GPIO_Pin_1
#define GPIO_LED_GREEN 		GPIO_Pin_2

#define GPIO_ADC_INPUT		GPIO_Pin_3

#define GPIO_SHIFT_DATA  	GPIO_Pin_6
#define GPIO_SHIFT_CLOCK 	GPIO_Pin_7
#define GPIO_SHIFT_RESET 	GPIO_Pin_8

typedef union
{
	struct
	{
		uint8_t flowRate  : 7;
		uint8_t adcStatus : 1;
	};
	uint8_t bits;
} controlADC_t;

#define MAX_ADC_VALUE      (128)
#define ADC_CONVERSION_ON  (1)
#define ADC_CONVERSION_OFF (!ADC_CONVERSION_ON)

#define SIMULATED_CARS 	   (19)
#define CAR_SPEED_MS 	   (500)

extern volatile controlADC_t controlADC;

void sys_GPIO_init( void );
void sys_ADC_init( void);


#endif // SYS_PROJECT_H
