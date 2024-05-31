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
#include <lookBusy.h>

#define STACK_SIZE 128

static void blinkTwice( LED* led );

void TaskA( void * argument);
void TaskB( void* argumet );
void TaskC( void* argumet );

// Create storage for a pointer to a semaphore
SemaphoreHandle_t semPtr = NULL;

int main(void)
{
    BaseType_t retVal;
    HWInit();
    SEGGER_SYSVIEW_Conf();
    NVIC_SetPriorityGrouping( 0 ); //ensure proper priority grouping for freeRTOS

    BlueLed.On();
    // Spin until the user starts the SystemView app, in Record mode
    while(SEGGER_SYSVIEW_IsStarted()==0){
        lookBusy(100);
    }
    BlueLed.Off();

    // Create a semaphore using the FreeRTOS Heap
    semPtr = xSemaphoreCreateBinary();
    assert_param(semPtr != NULL);

    retVal = xTaskCreate(TaskA, "TaskA", STACK_SIZE, NULL, tskIDLE_PRIORITY + 3, NULL) == pdPASS;
    assert_param(retVal == pdPASS);
    retVal = xTaskCreate(TaskB, "TaskB", STACK_SIZE, NULL, tskIDLE_PRIORITY + 2, NULL) == pdPASS;
    assert_param(retVal == pdPASS);
    retVal = xTaskCreate(TaskC, "TaskC", STACK_SIZE, NULL, tskIDLE_PRIORITY + 1, NULL) == pdPASS;
    assert_param(retVal == pdPASS);

    // Initial "give" of the semaphore
    xSemaphoreGive(semPtr);

    // Start the scheduler - shouldn't return unless there's a problem
    vTaskStartScheduler();

    // If you've wound up here, there is likely an issue with over-running the freeRTOS heap
    while(1)
    {
    }
}

/**
 * Task A is the highest priority task in the system.
 * It periodically takes the semaphore with a 200ms timeout.
 * If the semaphore is taken within 200 ms, the red LED is turned off,
   and blinkTwice() is called to blink the green LED.
 * If the semaphore is not taken within 200ms, the red LED is turned on
   (after TaskA has been given context)
 * At the end of each while-loop iteration, vTaskDelay is called.
 */
void TaskA( void* argument )
{
    uint32_t receivedCounter = 0;
    uint32_t timedoutCounter = 0;

    while(1)
    {

        // 'take' the semaphore with a 200ms timeout
        SEGGER_SYSVIEW_PrintfHost("attempt to take semPtr");
        if(xSemaphoreTake(semPtr, pdMS_TO_TICKS(200)) == pdPASS)
        {
            RedLed.Off();
            receivedCounter++;
            // In calling SEGGER_SYSVIEW_PrintfHost, the space between the %u and the closing
            // quote appears to be a necessary work-around for a bug in the API.
            SEGGER_SYSVIEW_PrintfHost("received semPtr: %u ", receivedCounter);
            blinkTwice(&GreenLed);
            xSemaphoreGive(semPtr);
        }
        else
        {
            // This code is called when the semaphore wasn't taken in time
            timedoutCounter++;
            SEGGER_SYSVIEW_PrintfHost("FAILED to receive semPtr in time: %u ", timedoutCounter);
            RedLed.On();
        }
        // Wait for a bit to let other tasks run
        // StmRand will return a random number between 5 and 30 (inclusive).
        vTaskDelay(StmRand(5,30));
    }
}

/**
 * This medium priority task just wakes up periodically and wastes time.
 */
void TaskB( void* argument )
{
    uint32_t counter = 0;
    float spinTime;

    while(1)
    {
        counter++;
        SEGGER_SYSVIEW_PrintfHost("starting iteration %u ", counter);
        vTaskDelay(pdMS_TO_TICKS(StmRand(10,25)));

        // Each for-loop iteration takes 1 ms of processing time.
        // The for-loop, as a whole, will use 30-75 ms of processing time.
        spinTime = StmRand(30,75);
        lookBusy(spinTime);
    }
}

/**
 * The lowest priority task in the system.
 * It is the same as TaskA, except:
   * It calls blinkTwice() to blink the blue LED
   * It does not call vTaskDelay().
 */
void TaskC( void* argument )
{
    uint32_t receivedCounter = 0;
    uint32_t timedoutCounter = 0;

    while(1)
    {
        // 'take' the semaphore with a 200mS timeout
        SEGGER_SYSVIEW_PrintfHost("attempt to take semPtr");
        if(xSemaphoreTake(semPtr, pdMS_TO_TICKS(200)) == pdPASS)
        {
            RedLed.Off();
            receivedCounter++;
            SEGGER_SYSVIEW_PrintfHost("received semPtr: %u ", receivedCounter);
            blinkTwice(&BlueLed);
            xSemaphoreGive(semPtr);
        }
        else
        {
            // This code is called when the semaphore wasn't taken in time
            timedoutCounter++;
            SEGGER_SYSVIEW_PrintfHost("FAILED to receive semPtr in time: %u ", timedoutCounter);
            RedLed.On();
        }
    }
}

/**
 * Blink the desired LED twice
 */
static void blinkTwice( LED* led )
{
    for(uint32_t i = 0; i < 2; i++)
    {
        led->On();
        vTaskDelay(pdMS_TO_TICKS(25));
        led->Off();
        vTaskDelay(pdMS_TO_TICKS(25));
    }
}
