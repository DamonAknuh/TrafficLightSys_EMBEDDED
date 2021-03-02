/*

*/

#include "sys_project.h"
#include "sys_timer.h"
#include "sys_tasks.h"

volatile controlADC_t controlADC;


/*-----------------------------------------------------------*/

int main(void)
{
	// ==> GPIO initialization
	sys_GPIO_init();

	// ==> ADC initialization
	sys_ADC_init();

	// ==> RTOS SW Timer initialization
	my_SW_TIM_Init();

	// ==> RTOS Task initialization
	my_TASK_Init();


	/* Start the tasks and timer running. */
	printf("~~~ PROGRAM START ~~~\n");
	printf("==> Starting Scheduler...\n");

	vTaskStartScheduler();

	printf("WARNING!! Main Exiting...\n");
	return 0;
}


/// =========================

void ADC_IRQHandler(void)
{
	uint16_t adc_value = 0;
	//portBASE_TYPE xHigherPriorityTaskWoken;

	// ==> Check if ADC EOC is indeed finished.
	if (ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC))
	{
		// ==> Retrieved value is between [0, 4096]
		adc_value = ADC_GetConversionValue(ADC1);

		controlADC.flowRate = adc_value / 32;
		controlADC.adcStatus = ADC_CONVERSION_OFF;
	}
}

/*-----------------------------------------------------------*/


/*-----------------------------------------------------------*/

void vApplicationMallocFailedHook( void )
{
	/* The malloc failed hook is enabled by setting
	configUSE_MALLOC_FAILED_HOOK to 1 in FreeRTOSConfig.h.

	Called if a call to pvPortMalloc() fails because there is insufficient
	free memory available in the FreeRTOS heap.  pvPortMalloc() is called
	internally by FreeRTOS API functions that create tasks, queues, software 
	timers, and semaphores.  The size of the FreeRTOS heap is set by the
	configTOTAL_HEAP_SIZE configuration constant in FreeRTOSConfig.h. */
	for( ;; );
}
/*-----------------------------------------------------------*/

void vApplicationStackOverflowHook( xTaskHandle pxTask, signed char *pcTaskName )
{
	( void ) pcTaskName;
	( void ) pxTask;

	/* Run time stack overflow checking is performed if
	configconfigCHECK_FOR_STACK_OVERFLOW is defined to 1 or 2.  This hook
	function is called if a stack overflow is detected.  pxCurrentTCB can be
	inspected in the debugger if the task name passed into this function is
	corrupt. */
	for( ;; );
}
/*-----------------------------------------------------------*/

void vApplicationIdleHook( void )
{
volatile size_t xFreeStackSpace;

	/* The idle task hook is enabled by setting configUSE_IDLE_HOOK to 1 in
	FreeRTOSConfig.h.

	This function is called on each cycle of the idle task.  In this case it
	does nothing useful, other than report the amount of FreeRTOS heap that
	remains unallocated. */
	xFreeStackSpace = xPortGetFreeHeapSize();

	if( xFreeStackSpace > 100 )
	{
		/* By now, the kernel has allocated everything it is going to, so
		if there is a lot of heap remaining unallocated then
		the value of configTOTAL_HEAP_SIZE in FreeRTOSConfig.h can be
		reduced accordingly. */
	}
}
/*-----------------------------------------------------------*/


