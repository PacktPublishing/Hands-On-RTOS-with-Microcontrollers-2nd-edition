/**

 * TITLE:  Chapter 7 example-program: task creation
 *
 * - This is an example program for the book:
 *   "Hands On RTOS with Microcontrollers" (2nd edition)

 ----------------------------------------------------

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
#include <BSP_Init.h>
#include <BSP_GPIO.h>
#include <task.h>
#include <SEGGER_SYSVIEW.h>
#include <lookBusy.h>

/**
 * 	Function prototypes
 */
void GreenTask(void *argument);
void BlueTask(void *argument);
void RedTask(void *argument);

// The address of blueTaskHandle will be passed to xTaskCreate.
// This global variable will be used by RedTask to delete BlueTask.
TaskHandle_t blueTaskHandle;

// In FreeRTOS, the stack size is specified in words.
// * Our processor's word size is 4 bytes
// * (128 * 4) = 512 bytes
#define STACK_SIZE 128

// Define the stack and the the task control block (TCB) for RedTask to use
static StackType_t RedTaskStack[STACK_SIZE];
static StaticTask_t RedTaskTCB;

int main(void)
{
    BaseType_t retVal;
    HWInit();
    SEGGER_SYSVIEW_Conf();
    NVIC_SetPriorityGrouping( 0 ); // ensure proper priority grouping for freeRTOS

    // Indicate waiting for SystemView to connect
    BlueLed.On();
    // Spin until the user starts the SystemView app, in Record mode
    while(SEGGER_SYSVIEW_IsStarted()==0)
    {
        lookBusy(100);
    }
    BlueLed.Off();

    // If the task isn't created successfully, main() will spin in the infinite while-loop.
    if (xTaskCreate(GreenTask, "GreenTask", STACK_SIZE, NULL, tskIDLE_PRIORITY + 2, NULL) != pdPASS)
    {
        while(1)
        {
        }
    }

    // Using an assert to ensure proper task creation
    retVal = xTaskCreate(BlueTask, "BlueTask", STACK_SIZE, NULL, tskIDLE_PRIORITY + 1, &blueTaskHandle);
    assert_param(retVal == pdPASS);

    // xTaskCreateStatic returns the task handle.
    // The function always passes because the function's memory was statically allocated.
    xTaskCreateStatic(	RedTask, "RedTask", STACK_SIZE, NULL, tskIDLE_PRIORITY + 1, RedTaskStack, &RedTaskTCB);

    // Start the scheduler. It should not return unless there's a problem.
    vTaskStartScheduler();

    // If you've wound up here, there is likely an issue with over-running the FreeRTOS heap
    while(1)
    {
    }
}

void GreenTask(void *argument)
{
    SEGGER_SYSVIEW_PrintfHost("GreenTask started");

    GreenLed.On();
    vTaskDelay(pdMS_TO_TICKS(1500));
    GreenLed.Off();

    SEGGER_SYSVIEW_PrintfHost("GreenTask is deleting itself");
    // A task can delete itself by passing NULL to vTaskDelete
    vTaskDelete(NULL);

    // The task never gets to here
    GreenLed.On();
}

void BlueTask( void* argument )
{
    while(1)
    {
        SEGGER_SYSVIEW_PrintfHost("BlueTask is starting a loop iteration");
        BlueLed.On();
        vTaskDelay(pdMS_TO_TICKS(200));
        BlueLed.Off();
        vTaskDelay(pdMS_TO_TICKS(200));
    }
}

void RedTask( void* argument )
{
    uint8_t firstIteration = 1;

    while(1)
    {
        SEGGER_SYSVIEW_PrintfHost("RedTask is starting a loop iteration");
        // Spin for 1 second of processor time
        lookBusy(1000);

        SEGGER_SYSVIEW_PrintfHost("RedTask is turning-on the red LED");
        RedLed.On();
        vTaskDelay(pdMS_TO_TICKS(500));
        RedLed.Off();
        vTaskDelay(pdMS_TO_TICKS(500));

        if(firstIteration == 1)
        {
            SEGGER_SYSVIEW_PrintfHost("RedTask is deleting BlueTask");
            // Tasks can delete one-another by passing the desired TaskHandle_t to vTaskDelete
            vTaskDelete(blueTaskHandle);
            firstIteration = 0;
        }
    }
}
