/*
 * sys_init.c
 *
 *  Created on: Feb 23, 2021
 *      Author: damonaknuh
 */

#include "sys_project.h"

void sys_GPIO_init( void );
void sys_ADC_init( void);

void sys_GPIO_init( void )
{
	/* Ensure all priority bits are assigned as preemption priority bits.
	http://www.freertos.org/RTOS-Cortex-M3-M4.html */
	NVIC_SetPriorityGrouping( 0 );

	// -->  Enable the GPIO AHB clock for PORT C
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);

	GPIO_InitTypeDef GPIO_InitStructureLED;
	GPIO_InitTypeDef GPIO_InitStructureShift;

	GPIO_InitStructureLED.GPIO_Pin 		= GPIO_LED_RED | GPIO_LED_YELLOW | GPIO_LED_GREEN;
	GPIO_InitStructureLED.GPIO_Mode 	= GPIO_Mode_OUT;
	GPIO_InitStructureLED.GPIO_OType 	= GPIO_OType_PP;
	GPIO_InitStructureLED.GPIO_PuPd 	= GPIO_PuPd_NOPULL;
	GPIO_InitStructureLED.GPIO_Speed    = GPIO_Speed_25MHz;
	GPIO_Init(GPIOC, &GPIO_InitStructureLED);

	GPIO_InitStructureShift.GPIO_Pin 	= GPIO_SHIFT_DATA | GPIO_SHIFT_CLOCK | GPIO_SHIFT_RESET;
	GPIO_InitStructureShift.GPIO_Mode 	= GPIO_Mode_OUT;
	GPIO_InitStructureShift.GPIO_OType 	= GPIO_OType_PP;
	GPIO_InitStructureShift.GPIO_PuPd 	= GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOC, &GPIO_InitStructureShift);

	// ==> Ensure Traffic LEDS are off
	GPIO_ResetBits(TRAFFIC_PORT, GPIO_LED_RED);
	GPIO_ResetBits(TRAFFIC_PORT, GPIO_LED_GREEN);
	GPIO_ResetBits(TRAFFIC_PORT, GPIO_LED_YELLOW);


	// ==> Reset Shift registers
	GPIO_ResetBits(TRAFFIC_PORT, GPIO_SHIFT_RESET);
	GPIO_SetBits(TRAFFIC_PORT, GPIO_SHIFT_RESET);

	RCC_AHB2PeriphClockCmd(RCC_AHB2Periph_RNG, ENABLE);
	RNG_Cmd(ENABLE);
}

void sys_ADC_init( void)
{
	// ==>  Enable the GPIO AHB clock for PORT C
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);
	// ==>  Enable the ADC Interface Clock
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1,  ENABLE);

	GPIO_InitTypeDef GPIO_InitStructureADC;
	ADC_InitTypeDef  ADC_InitStructure;

	// ==> Initialize the ADC GPIO pins to alternate function
	GPIO_InitStructureADC.GPIO_Pin 		= GPIO_Pin_3;
	GPIO_InitStructureADC.GPIO_PuPd 	= GPIO_PuPd_NOPULL;
	GPIO_InitStructureADC.GPIO_Mode 	= GPIO_Mode_AN;
	GPIO_InitStructureADC.GPIO_OType 	= GPIO_OType_PP;
	GPIO_InitStructureADC.GPIO_Speed 	= GPIO_Speed_25MHz;
	GPIO_Init(GPIOC, &GPIO_InitStructureADC);

	ADC1->CR1 |= ADC_CR1_EOCIE;
	//==> Set ADC completion priority to 1
	NVIC_SetPriority(ADC_IRQn, 0);
	// ==> Enable the ADC Completion ISR
	NVIC_EnableIRQ(ADC_IRQn);

	// ==> Initialize the ADC system
	ADC_InitStructure.ADC_NbrOfConversion 		= 1;
	ADC_InitStructure.ADC_ContinuousConvMode 	= DISABLE;
	ADC_InitStructure.ADC_ScanConvMode 			= DISABLE;
	ADC_InitStructure.ADC_ExternalTrigConv 		= DISABLE;
	ADC_InitStructure.ADC_ExternalTrigConvEdge 	= ADC_ExternalTrigConvEdge_None;
	ADC_InitStructure.ADC_Resolution 			= ADC_Resolution_12b;
	ADC_InitStructure.ADC_DataAlign 			= ADC_DataAlign_Right;
	ADC_Init(ADC1, &ADC_InitStructure);

	ADC_Cmd(ADC1, ENABLE);
	ADC_RegularChannelConfig(ADC1, ADC_Channel_13, 1, ADC_SampleTime_84Cycles);
}

