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

/*
 * This demo uses heap_1 for dynamic memory-allocation.  With heap_1,
 * memory may only be allocated and never freed.
 * If freeing memory is attempted, a hang will occur.
 *
 */

/**
 * 	Function prototypes
 */
void GreenTask(void *argument);
void BlueTask(void *argument);


// Recommended min stack size per task:
// (128 * 4 = 512 bytes)
#define STACK_SIZE 128


int main(void)
{
	HWInit();

	// Use retval to track the return value of xTaskCreate, issue asserts along the way
	// to stop execution if things don't go as planned
	BaseType_t retVal = pdPASS;

	// Create tasks, using dynamically-allocated memory
	retVal = xTaskCreate(GreenTask, "GreenTask", STACK_SIZE, NULL, tskIDLE_PRIORITY + 2, NULL);
	assert_param( retVal == pdPASS );

	retVal = xTaskCreate(BlueTask, "BlueTask", STACK_SIZE, NULL, tskIDLE_PRIORITY + 1, NULL);
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
     * "dynamic" memory, i.e., the heap.
     * However, heap_1 is being used, so dynamically-allocated memory cannot be freed.
     *
     * Calling vTaskDelete will result in a hang (or calling any other *Delete function
     * i.e. vQueueDelete, vEventGroupDelete, vSemaphoreDelete, vMutexDelete, etc)
     *
     * To observe this, un-comment the following line - you'll notice that the
     * green LED turns off.  Using a debugger, you'll also notice that
     * the green task is stopped by the function vPortFree() in Src/MemMang/heap_1.c
     *
     * When using heap_1.c  - it is best to adjust the FreeRTOSConfig.h to disable code
     * for vTaskDelete as follows:
     * #define INCLUDE_vTaskDelete 0
     *
     * This allows the linker to issue an undefined-reference error when
     * vTaskDelete is used.  This allows the problem to be found at link time, rather
     * than run time.
     **/
    //vTaskDelete(NULL);


    // If vTaskDelete(NULL) is called, the task will never get to here.
    while(1)
    {
        GreenLed.On();
        vTaskDelay(200 / portTICK_PERIOD_MS);
        GreenLed.Off();
        vTaskDelay(200 / portTICK_PERIOD_MS);
    }

}

void BlueTask( void* argument )
{
	while(1)
	{
		BlueLed.On();
		vTaskDelay(200 / portTICK_PERIOD_MS);
		BlueLed.Off();
		vTaskDelay(200 / portTICK_PERIOD_MS);
	}
}

