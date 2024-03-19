/*

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
#include <Nucleo_F767ZI_Init.h>
#include <stm32f7xx_hal.h>
#include <Nucleo_F767ZI_GPIO.h>
#include <task.h>
#include <stdlib.h>

/**
* Individual heap implementations are located at: Src\emMang\heap_x.c.
* Only one heap_x.c source file can be used in a build-configuration, otherwise linker errors
* will occur due to multiple definitions of the same functions.
*
**/

/*
 * This demo uses heap_1 for dynamic memory-allocation.
 * It creates a "statically" allocated task, to illustrate
 * that it is possible to use vTaskDelete to remove the task from the scheduler.
 *
 */

/**
 * 	function prototypes
 */
void GreenTask(void *argument);
void BlueTask(void *argument);


// Recommended min stack size per task:
// (128 * 4 = 512 bytes)
#define STACK_SIZE 128

StackType_t GreenTaskStack[STACK_SIZE];
StaticTask_t GreenTaskTCB;

int main(void)
{
	HWInit();

	TaskHandle_t greenHandle = NULL;

	greenHandle = xTaskCreateStatic(	GreenTask, "GreenTask", STACK_SIZE,
										NULL, tskIDLE_PRIORITY + 2,
										GreenTaskStack, &GreenTaskTCB);
	assert_param( greenHandle != NULL);


	BaseType_t retVal = xTaskCreate(	BlueTask, "BlueTask", STACK_SIZE,
											NULL, tskIDLE_PRIORITY + 2, NULL);
	assert_param( retVal == pdPASS );

	// Start the scheduler - shouldn't return unless there's a problem
	vTaskStartScheduler();

	// If you've wound up here, there is likely an issue with over-running the FreeRTOS heap
	while(1)
	{
	}
}

void GreenTask(void *argument)
{

    GreenLed.On();
	vTaskDelay(1500/ portTICK_PERIOD_MS);
	GreenLed.Off();

	/**
	 * In this example, the GreenTask has been created using
	 * "static" memory.  Calling vTaskDelete will remove
	 * the task from being executed by the scheduler, but will not
	 * have any affect on the memory that was allocated for the
	 * task's stack (GreenTaskStack) or TCB (GreenTaskTCB).
	 * After a call to vTaskDelete, that memory can be safely
	 * used for other purposes. That memory will no longer be accessed by
	 * FreeRTOS.
	 **/
	vTaskDelete(NULL);

	// Task never gets here
	GreenLed.On();
}

void BlueTask( void* argument )
{
	while(1)
	{
		BlueLed.On();
		vTaskDelay(500 / portTICK_PERIOD_MS);
		BlueLed.Off();
		vTaskDelay(500 / portTICK_PERIOD_MS);
	}
}

