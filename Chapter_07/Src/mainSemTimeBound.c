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
#include <task.h>
#include <semphr.h>
#include <SEGGER_SYSVIEW.h>
#include <BSP_GPIO.h>
#include <BSP_Init.h>

#define STACK_SIZE 128

static void greenBlink( void );
static void blueTripleBlink( void );

void GreenTaskA( void * argument);
void TaskB( void* argumet );

// Create storage for a pointer to a semaphore
SemaphoreHandle_t semPtr = NULL;

int main(void)
{
	BaseType_t retVal;
	HWInit();
	SEGGER_SYSVIEW_Conf();
	NVIC_SetPriorityGrouping( 0 );	// Ensure proper priority grouping for freeRTOS

	// Create a semaphore using the FreeRTOS Heap
	semPtr = xSemaphoreCreateBinary();
	assert_param(semPtr != NULL);

	// Create TaskA as a higher priority than TaskB.  In this example, this isn't strictly necessary since the tasks
	// spend nearly all of their time blocked
	retVal = xTaskCreate(GreenTaskA, "GreenTaskA", STACK_SIZE, NULL, tskIDLE_PRIORITY + 2, NULL) == pdPASS;
	assert_param(retVal == pdPASS);

	// Using an assert to ensure proper task creation
	retVal = xTaskCreate(TaskB, "TaskB", STACK_SIZE, NULL, tskIDLE_PRIORITY + 1, NULL) == pdPASS;
	assert_param(retVal == pdPASS);

	// Start the scheduler - shouldn't return unless there's a problem
	vTaskStartScheduler();

	// If you've wound up here, there is likely an issue with over-running the freeRTOS heap
	while(1)
	{
	}
}

/**
 * Task A periodically 'gives' semaphorePtr.  This version
   has some variability in how often it will give the semaphore
 * NOTES:
 * - This semaphore isn't "given" to any task specifically
 * - Giving the semaphore doesn't prevent taskA from continuing to run.
 * - Notice the green LED continues to blink at all times.
 */
void GreenTaskA( void* argument )
{
	uint_fast8_t count = 0;

	while(1)
	{
	    // Get a random number between 3 and 9, inclusive.
		uint8_t numLoops = StmRand(3,9);
		if(++count >= numLoops)
		{
			count = 0;
			SEGGER_SYSVIEW_PrintfHost("Task A (green LED) gives semPtr");
			xSemaphoreGive(semPtr);
		}
		greenBlink();
	}
}

/**
 * Wait to receive semPtr and triple blink the Blue LED.
 * If the semaphore isn't available within 500 ms, then
   turn on the RED LED until the semaphore is available.
 */
void TaskB( void* argument )
{
	while(1)
	{
		// 'take' the semaphore with a 500mS timeout
		SEGGER_SYSVIEW_PrintfHost("attempt to take semPtr");
		if(xSemaphoreTake(semPtr, pdMS_TO_TICKS(500)) == pdPASS)
		{
			RedLed.Off();
			SEGGER_SYSVIEW_PrintfHost("received semPtr");
			blueTripleBlink();
		}
		else
		{
			// This code is run when the semaphore wasn't taken in time
			SEGGER_SYSVIEW_PrintfHost("FAILED to receive semPtr in time");
			RedLed.On();
		}
	}
}

/**
 * Blink the Green LED once using vTaskDelay
 */
static void greenBlink( void )
{
	GreenLed.On();
	vTaskDelay(pdMS_TO_TICKS(100));
	GreenLed.Off();
	vTaskDelay(pdMS_TO_TICKS(100));
}

/**
 * blink the Blue LED 3 times in rapid succession using vtaskDelay
 */
static void blueTripleBlink( void )
{
	//triple blink the Blue LED
	for(uint_fast8_t i = 0; i < 3; i++)
	{
		BlueLed.On();
		vTaskDelay(pdMS_TO_TICKS(50));
		BlueLed.Off();
		vTaskDelay(pdMS_TO_TICKS(50));
	}
}
