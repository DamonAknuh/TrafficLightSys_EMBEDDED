


#include "sys_project.h"
#include "sys_timer.h"
#include "sys_tasks.h"

volatile uint8_t  g_CurrentLState;
EventGroupHandle_t xEVT_FSM_Transition;

void util_display(uint32_t carTraffic)
{
	int16_t i;

	// ==> Reset SPC Output
	GPIO_ResetBits(TRAFFIC_PORT, GPIO_SHIFT_RESET);
	GPIO_SetBits(TRAFFIC_PORT, GPIO_SHIFT_RESET);

	// ==> Loop over the carTraffic integer
	//      * Add MSB of carTraffic to SPC in LIFO order.
	//     	* Fast so human eye only discerns final result.
	for(i = (SIMULATED_CARS - 1); i >= 0; i--)
	{
		if (1 & (carTraffic >> i))
		{
			GPIO_SetBits(TRAFFIC_PORT, GPIO_SHIFT_DATA);

		}
		else
		{
			GPIO_ResetBits(TRAFFIC_PORT, GPIO_SHIFT_DATA);
		}

		GPIO_SetBits(TRAFFIC_PORT, GPIO_SHIFT_CLOCK );
		GPIO_ResetBits(TRAFFIC_PORT, GPIO_SHIFT_CLOCK);
	}
}

void xTrafficSimulator(void *pvParameters)
{
	uint8_t  addCar		= 0;
	uint8_t  carQueue 	= 0;
	uint32_t preLightTraffic;
	uint32_t postLightTraffic;
	uint32_t carTraffic = 0x0;

	printf("==> Starting TASK: FLOW SAMPLER\n");

	while(1)
	{
		// ==> Wait for RNG to be ready
		while(!RNG_GetFlagStatus(RNG_FLAG_DRDY));

		// ==> Add a car based on a ratio
		addCar = (RNG->DR % MAX_ADC_VALUE)  > (MAX_ADC_VALUE - controlADC.flowRate);

		carTraffic |= addCar;

		// ==> Loop to display the Cars on the LEDS.
		util_display(carTraffic);

		// ==> Post process to determine where the cars are depending:
		// 			* GREEN || YELLOW: all cars move
		//			* RED: Cars in-front of light bunch, cars after red light go.
		if(EVAL_STATE(g_CurrentLState, RED_STATE))
		{
			// ==> Construct queue of cars waiting at redLight.
			for (int16_t i = 7; i >= 0; i--)
			{
				if (0x1 & (carTraffic >> i))
				{
					// ==> Add car to red light queue.
					carQueue |= 1 << i;
				}
				else
				{
					// ==> Space detected for cars to move up.
					break;
				}
			}


			preLightTraffic  = ((carTraffic << 1) | carQueue) & 0xFF;

			postLightTraffic = (carTraffic & 0x3FF00) << 1;

			carTraffic = preLightTraffic | postLightTraffic;
		}
		else
		{
			// ==> Release red light car queue.
			carQueue = 0;

			// ==> All traffic moves normally.
			carTraffic = (carTraffic << 1) & 0x0007FFFF;
		}

		// ==> Task delay to simulate car speed.
		vTaskDelay(pdMS_TO_TICKS(CAR_SPEED_MS));

	}
	printf("==> TASK ENDED: FLOW SAMPLER\n");
}

void xTrafficLightFSM(void *pvParameters)
{
	EventBits_t evtBits;
	uint32_t lightTimerPeriod;

	printf("==> Starting TASK: Traffic Generator\n");

	// ==> Initialize to Green Light
	xEventGroupSetBits(xEVT_FSM_Transition,	GREEN_STATE);

	while(1)
	{
		// ==> task waits for the light timer callback to finish and assert next state.
		evtBits = xEventGroupWaitBits(xEVT_FSM_Transition,
					GREEN_STATE | RED_STATE | YELLOW_STATE,
					pdTRUE,
					pdFALSE,
					MAX_EVENT_WAIT);

		if (EVAL_STATE(evtBits, GREEN_STATE))
		{
			printf("Light State: GREEN\n");

			g_CurrentLState = GREEN_STATE;

			// ==> Turn Off the RED LED and turn ON the GREEN
			GPIO_ResetBits(TRAFFIC_PORT, GPIO_LED_RED);
			GPIO_SetBits(TRAFFIC_PORT, 	 GPIO_LED_GREEN);

			// ==> Set GREEN light period based on traffic flow
			lightTimerPeriod =  GREEN_LED_0_MS - (MS_CHANGE_PER_ADC * controlADC.flowRate);

			xTimerChangePeriod(xTIM_Light_Timer,
					lightTimerPeriod,
					0);

			xTimerStart(xTIM_Light_Timer, 0);
		}
		if (EVAL_STATE(evtBits, YELLOW_STATE))
		{
			printf("Light State: YELLOW\n");

			g_CurrentLState = YELLOW_STATE;

			// ==> Turn Off the RED LED and turn ON the GREEN
			GPIO_ResetBits(TRAFFIC_PORT, GPIO_LED_GREEN);
			GPIO_SetBits(TRAFFIC_PORT, 	 GPIO_LED_YELLOW);

			lightTimerPeriod = YELLOW_L_MS;

			xTimerChangePeriod(xTIM_Light_Timer,
					lightTimerPeriod,
					0);

			xTimerStart(xTIM_Light_Timer, 0);
		}
		if (EVAL_STATE(evtBits, RED_STATE))
		{
			printf("Light State: RED\n");

			g_CurrentLState = RED_STATE;

			// ==> Turn Off the RED LED and turn ON the GREEN
			GPIO_ResetBits(TRAFFIC_PORT, GPIO_LED_YELLOW);
			GPIO_SetBits(TRAFFIC_PORT, 	 GPIO_LED_RED);

			// ==> Set RED light period based on traffic flow
			lightTimerPeriod = (MS_CHANGE_PER_ADC * controlADC.flowRate) + RED_LED_0_MS_0;

			xTimerChangePeriod(xTIM_Light_Timer,
					lightTimerPeriod,
					0);

			xTimerStart(xTIM_Light_Timer, 0);

		}
	}
	printf("==> TASK ENDED: Traffic Generator\n");
}

void my_TASK_Init(void)
{
	xEVT_FSM_Transition = xEventGroupCreate();

	xTaskCreate(xTrafficSimulator, "tSimulator", 	configMINIMAL_STACK_SIZE, NULL, TASK_PR_1, NULL);
	xTaskCreate(xTrafficLightFSM,  "lightFSM", 	    configMINIMAL_STACK_SIZE, NULL, TASK_PR_2, NULL);
}



