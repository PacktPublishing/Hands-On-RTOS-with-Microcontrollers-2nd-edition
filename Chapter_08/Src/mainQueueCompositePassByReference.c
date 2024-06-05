/**

---------------------------------------------------------------------------------------

 Copyright for modifications to the original program:
 * Copyright (c) by PACKT, 2022, under the MIT License.
 * - See the code-repository's license statement for more information:
 * - https://github.com/PacktPublishing/Hands-On-RTOS-with-Microcontrollers-2nd-edition

 The original program is in the code-repository:
 - https://github.com/PacktPublishing/Hands-On-RTOS-with-Microcontrollers
 Copyright for the original program:
 * MIT License
 *
 * Copyright (c) 2019 Brian Amos
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 */

#include <FreeRTOS.h>
#include <BSP_GPIO.h>
#include <queue.h>
#include <SEGGER_SYSVIEW.h>
#include <BSP_Init.h>

/*********************************************
 * A simple demonstration of using queues across
 * multiple tasks with pass by reference.
 * This time, a pointer to a struct
 * is copied into the queue
 *********************************************/


#define STACK_SIZE 128

// Maximum length of the message struct
#define MAX_MSG_LEN 256

/**
 * Define a larger structure to also specify a message
 */
typedef struct
{
	uint32_t redLEDState : 1;
	uint32_t blueLEDState : 1;
	uint32_t greenLEDState : 1;
	uint32_t msDelayTime;		// Minimum number of ms to remain in this state
	char message[MAX_MSG_LEN];	// An array for storing strings of up to 256 chars
} LedStates_t;

/**
 * Below, we define two instances of the LedStates_t struct.  Since these are globals
 * they will always be available for use.
 */
// Red ON, blue OFF, green OFF, 1000ms delay, msg
static const LedStates_t ledState1 = {1, 0, 0, 1000, "The quick brown fox jumped over the lazy dog. Only the Red LED is on."};

// Red OFF, blue ON, green OFF, 1000ms delay, msg
static const LedStates_t ledState2 = {0, 1, 0, 1000, "Another string. Only the Blue LED is on"};

void recvTask( void* NotUsed );
void sendingTask( void* NotUsed );

// This is a handle for the queue that will be used by
// recvTask and sendingTask
static QueueHandle_t ledCmdQueue = NULL;

int main(void)
{
	HWInit();
	SEGGER_SYSVIEW_Conf();
	NVIC_SetPriorityGrouping( 0 ); // ensure proper priority grouping for freeRTOS

	// Setup tasks, making sure they have been properly created before moving on
	BaseType_t retVal;
	retVal = xTaskCreate(recvTask, "recvTask", STACK_SIZE, NULL, tskIDLE_PRIORITY + 2, NULL);
	assert_param(retVal == pdPASS);
	retVal = xTaskCreate(sendingTask, "sendingTask", STACK_SIZE, NULL, tskIDLE_PRIORITY + 1, NULL);
	assert_param(retVal == pdPASS);

	/**
	 * Create a queue that can store up to 8 LedStates_t struct POINTERS.
	 * It is very important to note that each element of this queue only
	 * contains enough space for storing a POINTER to LedStates_t, not
	 * the entire struct.
	 *
	 * In Ozone's Global Data view, take a look at ledCmdQueue->uxItemSize.
	 * Notice how it differs from the value in mainQueueLargeCompositePassByValue.c
	 *
	 */
	ledCmdQueue = xQueueCreate(8, sizeof(LedStates_t*));
	assert_param(ledCmdQueue != NULL);

	// Start the scheduler - shouldn't return unless there's a problem
	vTaskStartScheduler();

	// if you've wound up here, there is likely an issue with over-running the freeRTOS heap
	while(1)
	{
	}
}

/**
 * recvTask receives items from queue, of type LedStates_t pointer.
 */
void recvTask( void* NotUsed )
{
	LedStates_t* nextCmd;

	while(1)
	{
		xQueueReceive(ledCmdQueue, &nextCmd, portMAX_DELAY);

        if(nextCmd->redLEDState == 1)
            RedLed.On();
        else
            RedLed.Off();

        if(nextCmd->blueLEDState == 1)
            BlueLed.On();
        else
            BlueLed.Off();

        if(nextCmd->greenLEDState == 1)
            GreenLed.On();
        else
            GreenLed.Off();

		vTaskDelay(pdMS_TO_TICKS(nextCmd->msDelayTime));
	}
}

/**
 * sendingTask sends items to the queue, of type LedStates_t pointer.
 *
 */
void sendingTask( void* NotUsed )
{
	const LedStates_t * state1Ptr = &ledState1;
	const LedStates_t * state2Ptr = &ledState2;

	while(1)
	{
		xQueueSend(ledCmdQueue, &state1Ptr, portMAX_DELAY);
		xQueueSend(ledCmdQueue, &state2Ptr, portMAX_DELAY);
	}
}

