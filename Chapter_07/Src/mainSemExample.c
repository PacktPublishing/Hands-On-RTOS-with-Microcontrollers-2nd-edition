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

void GreenTaskA( void * argument);
void BlueTaskB( void* argumet );

// Create storage for a pointer to a semaphore
SemaphoreHandle_t semPtr = NULL;

int main(void)
{
	BaseType_t retVal;
	HWInit();
	SEGGER_SYSVIEW_Conf();
	NVIC_SetPriorityGrouping( 0 );	//ensure proper priority grouping for freeRTOS

	// Create a semaphore using the FreeRTOS Heap
	semPtr = xSemaphoreCreateBinary();
    // Ensure the pointer is valid (semaphore created successfully)
	assert_param(semPtr != NULL);

	// Create TaskA as a higher priority than TaskB.  In this example, this isn't strictly necessary since the tasks
	// spend nearly all of their time blocked
	retVal = xTaskCreate(GreenTaskA, "GreenTaskA", STACK_SIZE, NULL, tskIDLE_PRIORITY + 2, NULL) == pdPASS;
	assert_param(retVal == pdPASS);

	// Using an assert to ensure proper task creation
	retVal = xTaskCreate(BlueTaskB, "BlueTaskB", STACK_SIZE, NULL, tskIDLE_PRIORITY + 1, NULL) == pdPASS;
	assert_param(retVal == pdPASS);

	// Start the scheduler - shouldn't return unless there's a problem
	vTaskStartScheduler();

	// If you've wound up here, there is likely an issue with over-running the freeRTOS heap
	while(1)
	{
	}
}

/**
 * GreenTaskA periodically 'gives' semaphorePtr
 * NOTES:
 * - This semaphore isn't "given" to any task specifically
 * - Giving the semaphore doesn't prevent GreenTaskA from continuing to run.
 * - Note the green LED continues to blink at all times
 */
void GreenTaskA( void* argument )
{
	uint_fast8_t count = 0;
	while(1)
	{
		// Every 5 times through the loop, give the semaphore
		if(++count >= 5)
		{
			count = 0;
			SEGGER_SYSVIEW_PrintfHost("GreenTaskA gives semPtr");
			xSemaphoreGive(semPtr);
		}
		GreenLed.On();
		vTaskDelay(pdMS_TO_TICKS(100));
		GreenLed.Off();
		vTaskDelay(pdMS_TO_TICKS(100));
	}
}

/**
 * BlueTaskB waits to receive semPtr, then triple-blinks the blue LED
 */
void BlueTaskB( void* argument )
{
	while(1)
	{
		// 'take' the semaphore with no timeout.
	    // * In our system, FreeRTOSConfig.h specifies "#define INCLUDE_vTaskSuspend 1".
	    // * So, in xSemaphoreTake, portMAX_DELAY specifies an indefinite wait.
		SEGGER_SYSVIEW_PrintfHost("BlueTaskB attempts to take semPtr");
		if(xSemaphoreTake(semPtr, portMAX_DELAY) == pdPASS)
		{
			SEGGER_SYSVIEW_PrintfHost("BlueTaskB received semPtr");
			// Triple-blink the Blue LED
			for(uint_fast8_t i = 0; i < 3; i++)
			{
				BlueLed.On();
				vTaskDelay(pdMS_TO_TICKS(50));
				BlueLed.Off();
				vTaskDelay(pdMS_TO_TICKS(50));
			}
		}
//		else
//		{
//			This is the code that would be executed if we timed-out waiting for
//			the semaphore to be given.
//		}
	}
}
