


#include "sys_project.h"
#include "sys_timer.h"
#include "sys_tasks.h"

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
	uint8_t  flowRate;
	uint8_t  lightState;
	uint8_t  addCar		= 0;
	uint8_t  carQueue 	= 0;
	uint8_t  minimumFlow =0;
	uint32_t preLightTraffic;
	uint32_t postLightTraffic;
	uint32_t carTraffic = 0x0;

	printf("==> Starting TASK: TRAFFIC SIMUALTOR\n");

	while(1)
	{
		// ==> Wait for RNG to be ready
		while(!RNG_GetFlagStatus(RNG_FLAG_DRDY));

		// ==> Grab the current Flowrate from the flow rate queue.
		//		 only wait 50mS to prevent undue latency.
		xQueuePeek(xQ_FlowRate,  	&flowRate, 	 pdMS_TO_TICKS(50));
		xQueuePeek(xQ_LightState,  	&lightState, pdMS_TO_TICKS(100));

		// ==> Add a car based on the traffic flow rate.
		addCar = (RNG->DR % MAX_ADC_VALUE)  > (MAX_ADC_VALUE - flowRate);
		carTraffic |= ((addCar) || !(minimumFlow % MINIMUM_FLOW));

		// ==> Loop to display the Cars on the LEDS.
		util_display(carTraffic);

		// ==> Post process to determine where the cars are depending:
		// 			* GREEN : all cars move
		//			* RED|| YELLOW : Cars in-front of light bunch, cars after red light go.
		if (GREEN_STATE == lightState)
		{
			// ==> Release red light car queue.
			carQueue = 0;

			// ==> All traffic moves normally.
			carTraffic = (carTraffic << 1) & 0x0007FFFF;
		}
		else
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

		minimumFlow = (minimumFlow + 1) % (MINIMUM_FLOW);

		// ==> Task delay to simulate car speed.
		vTaskDelay(pdMS_TO_TICKS(CAR_SPEED_MS));
	}

	printf("==> TASK ENDED: TRAFFIC SIMULATOR\n");
}

void xTrafficLightFSM(void *pvParameters)
{
	uint8_t 	flowRate;
	uint32_t 	lightTimerPeriod;
	EventBits_t evtBits;

	printf("==> Starting TASK: TRAFFIC FSM\n");
	while(1)
	{
		// ==> task waits for the light timer callback to finish and assert next state.
		evtBits = xEventGroupWaitBits(xEVT_FSM_Transition,
					GREEN_STATE | RED_STATE | YELLOW_STATE,
					pdTRUE,
					pdFALSE,
					MAX_EVENT_WAIT);

		// ==> Peek into the Queue and grab the flow rate.
		xQueuePeek(xQ_FlowRate, &flowRate, pdMS_TO_TICKS(1000));

		if (EVAL_STATE(evtBits, GREEN_STATE))
		{
			printf("Light State: GREEN\n");

			// ==> Turn Off the RED LED and turn ON the GREEN
			GPIO_ResetBits(TRAFFIC_PORT, GPIO_LED_RED);
			GPIO_SetBits(TRAFFIC_PORT, 	 GPIO_LED_GREEN);

			// ==> Set GREEN light period based on traffic flow
			lightTimerPeriod = pdMS_TO_TICKS(GREEN_LED_0_MS - (MS_CHANGE_PER_ADC * flowRate));

			xTimerChangePeriod(xTIM_Light_Timer,
					lightTimerPeriod,
					0);

			xTimerStart(xTIM_Light_Timer, 0);
		}
		if (EVAL_STATE(evtBits, YELLOW_STATE))
		{
			printf("Light State: YELLOW\n");

			// ==> Turn Off the RED LED and turn ON the GREEN
			GPIO_ResetBits(TRAFFIC_PORT, GPIO_LED_GREEN);
			GPIO_SetBits(TRAFFIC_PORT, 	 GPIO_LED_YELLOW);

			lightTimerPeriod = pdMS_TO_TICKS(YELLOW_L_MS);

			xTimerChangePeriod(xTIM_Light_Timer,
					lightTimerPeriod,
					0);

			xTimerStart(xTIM_Light_Timer, 0);
		}
		if (EVAL_STATE(evtBits, RED_STATE))
		{
			printf("Light State: RED\n");

			// ==> Turn Off the RED LED and turn ON the GREEN
			GPIO_ResetBits(TRAFFIC_PORT, GPIO_LED_YELLOW);
			GPIO_SetBits(TRAFFIC_PORT, 	 GPIO_LED_RED);

			// ==> Set RED light period based on traffic flow
			lightTimerPeriod = pdMS_TO_TICKS((MS_CHANGE_PER_ADC * flowRate) + RED_LED_0_MS_0);

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
	uint8_t lightStateInit = GREEN_STATE;

	xQ_LightState = xQueueCreate(QUEUE_LENGTH, QUEUE_ITEM_SIZE);
	xQueueSend(xQ_LightState, &lightStateInit,  0);

	xEVT_FSM_Transition = xEventGroupCreate();
	// ==> Initialize to Green Light
	xEventGroupSetBits(xEVT_FSM_Transition,	GREEN_STATE);

	xTaskCreate(xTrafficSimulator, "TRAFFIC_SIM", 	configMINIMAL_STACK_SIZE, NULL, TASK_PR_1, NULL);
	xTaskCreate(xTrafficLightFSM,  "TRAFFIC_FSM", 	    configMINIMAL_STACK_SIZE, NULL, TASK_PR_2, NULL);
}

