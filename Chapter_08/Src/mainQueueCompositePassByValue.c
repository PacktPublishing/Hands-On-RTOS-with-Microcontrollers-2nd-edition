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
 * multiple tasks with pass by value.
 * This time, a large struct is copied into
 * the queue.
 *********************************************/

#define STACK_SIZE 128

/**
 * Define a larger structure to also specify the number of
 * milliseconds this state should last.
 * Notice the difference in the queue's item-size.
 */
typedef struct
{
    uint8_t redLEDState : 1;    // Specify this variable as 1 bit wide
    uint8_t blueLEDState : 1;   // Specify this variable as 1 bit wide
    uint8_t greenLEDState : 1;  // Specify this variable as 1 bit wide
    uint32_t msDelayTime;   // Minimum number of ms to remain in this state
} LedStates_t;

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
    retVal = xTaskCreate(recvTask, "recvTask", STACK_SIZE, NULL, (tskIDLE_PRIORITY + 1), NULL);
    assert_param(retVal == pdPASS);
    retVal = xTaskCreate(sendingTask, "sendingTask", STACK_SIZE, NULL, (configMAX_PRIORITIES - 1), NULL);
    assert_param(retVal == pdPASS);

    /**
     * Create a queue that can store up to 8 copies of the struct.
     * Using sizeof allows us to modify the struct and have the queue-item's
     * storage sized at compile time
     */
    ledCmdQueue = xQueueCreate(8, sizeof(LedStates_t));
    assert_param(ledCmdQueue != NULL);

    // Start the scheduler - shouldn't return unless there's a problem
    vTaskStartScheduler();

    // If you've wound up here, there is likely an issue with over-running the freeRTOS heap
    while(1)
    {
    }
}

/**
 * This receive task watches the queue for a new command added to it
 */
void recvTask( void* NotUsed )
{
    LedStates_t nextCmd;

    while(1)
    {
        xQueueReceive(ledCmdQueue, &nextCmd, portMAX_DELAY);

        if(nextCmd.redLEDState == 1)
            RedLed.On();
        else
            RedLed.Off();

        if(nextCmd.blueLEDState == 1)
            BlueLed.On();
        else
            BlueLed.Off();

        if(nextCmd.greenLEDState == 1)
            GreenLed.On();
        else
            GreenLed.Off();

        vTaskDelay(pdMS_TO_TICKS(nextCmd.msDelayTime));
    }
}

/**
 * sendingTask modifies a single nextStates variable
 * and passes it to the queue.
 * Each time the variable is passed to the queue, its
 * value is copied into the queue, which is allowed to
 * fill to capacity.
 */
void sendingTask( void* NotUsed )
{
    // A single instance of nextStates is defined here
    LedStates_t nextStates;

    while(1)
    {
        nextStates.redLEDState = 1;
        nextStates.greenLEDState = 1;
        nextStates.blueLEDState = 1;
        nextStates.msDelayTime = 500;
        xQueueSend(ledCmdQueue, &nextStates, portMAX_DELAY);

        nextStates.greenLEDState = 0;  // Turn-off the green LED
        xQueueSend(ledCmdQueue, &nextStates, portMAX_DELAY);

        nextStates.blueLEDState = 0;  // Turn-off the blue LED
        xQueueSend(ledCmdQueue, &nextStates, portMAX_DELAY);

        nextStates.redLEDState = 0;  // Turn-off the red LED
        nextStates.msDelayTime = 1000;
        xQueueSend(ledCmdQueue, &nextStates, portMAX_DELAY);
    }

}

