/*
 * sys_tasks.h
 *
 *  Created on: Feb 28, 2021
 *      Author: damonaknuh
 */

#ifndef SYS_TASKS_H_
#define SYS_TASKS_H_

#include "sys_project.h"

#define MAX_EVENT_WAIT (6000 / portTICK_PERIOD_MS)

typedef enum
{
    GREEN_STATE          = 0b00000001, // 1
	YELLOW_STATE		 = 0b00000010,
	RED_STATE			 = 0b00000100,
	MAX_STATE     		 = 0b00001000,
} systemStates_e;

extern volatile uint8_t  g_CurrentLState;
extern EventGroupHandle_t xEVT_FSM_Transition;;

// == > FSM UTILITY MACROS
#define GET_CUR_STATE()			  (g_CurrentLState)
#define TRIGGER_STATE(STATE)      (g_CurrentLState = STATE)
#define TOGGLE_STATE(STATE)       (g_CurrentLState ^= STATE)
#define EVAL_STATE(reg, STATE)    (reg & STATE)


#define TASK_PR_1 (tskIDLE_PRIORITY + 1U)
#define TASK_PR_2 (tskIDLE_PRIORITY + 2U)
#define TASK_PR_3 (tskIDLE_PRIORITY + 3U

void my_TASK_Init(void);

#endif /* SYS_TASKS_H_ */
