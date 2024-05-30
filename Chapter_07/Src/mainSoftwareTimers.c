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
#include <BSP_GPIO.h>
#include <task.h>
#include <semphr.h>
#include <timers.h>
#include <SEGGER_SYSVIEW.h>
#include <BSP_Init.h>
#include <lookBusy.h>

#define STACK_SIZE 128

void taskStartTimers( void * argument);

void oneShotCallBack( TimerHandle_t xTimer );
void repeatCallBack( TimerHandle_t xTimer );

int main(void)
{
	BaseType_t retVal;
	HWInit();
	SEGGER_SYSVIEW_Conf();
	NVIC_SetPriorityGrouping( 0 );	//ensure proper priority grouping for FreeRTOS

	// Create the task that will start the timers.
	// Set the task's priority to be higher than the timer-task: (configTIMER_TASK_PRIORITY + 1).
	retVal = xTaskCreate(	taskStartTimers, "startTimersTask",
	                    	STACK_SIZE, NULL, (configTIMER_TASK_PRIORITY + 1), NULL) == pdPASS;
	assert_param(retVal == pdPASS);

	// Start the scheduler - shouldn't return unless there's a problem
	vTaskStartScheduler();

	// If you've wound up here, there is likely an issue with over-running the freeRTOS heap
	while(1)
	{
	}
}

// Task used to start the timers
void taskStartTimers( void* argument )
{
    // Indicate taskStartTimers started
    RedLed.On();
    // Spin until the user starts the SystemView app, in Record mode
    while(SEGGER_SYSVIEW_IsStarted()==0){
        lookBusy(100);
    }
    RedLed.Off();

	SEGGER_SYSVIEW_PrintfHost("taskStartTimers: starting");

    //
    // Create the one-shot timer, and start it
    //

	// Start with Blue LED on - it will be turned off after one-shot fires
	BlueLed.On();
	SEGGER_SYSVIEW_PrintfHost("taskStartTimers: blue LED on");
	TimerHandle_t oneShotHandle =
		xTimerCreate(	"myOneShotTimer",		//name for timer
						pdMS_TO_TICKS(2200),	//period of timer in ticks
						pdFALSE,				//auto-reload flag
						NULL,					//unique ID for timer
						oneShotCallBack);		//callback function
	assert_param(oneShotHandle != NULL);

	SEGGER_SYSVIEW_PrintfHost("taskStartTimers: one-shot timer started (turns off blue LED)");
	xTimerStart(oneShotHandle, 0);


    //
    // Create the repeat-timer, and start it
    //

    TimerHandle_t repeatHandle =
        xTimerCreate(   "myRepeatTimer",        //name for timer
                        pdMS_TO_TICKS(500),    //period of timer in ticks
                        pdTRUE,                //auto-reload flag
                        NULL,                  //unique ID for timer
                        repeatCallBack);       //callback function
    assert_param(repeatHandle != NULL);

    SEGGER_SYSVIEW_PrintfHost("taskStartTimers: repeating-timer started (blinks the green LED)");
    xTimerStart(repeatHandle, 0);


	// The task deletes itself
    SEGGER_SYSVIEW_PrintfHost("taskStartTimers: deleting itself");
	vTaskDelete(NULL);

    // The task never gets to here
	while(1)
	{
	}

}


void oneShotCallBack( TimerHandle_t xTimer )
{
	SEGGER_SYSVIEW_PrintfHost("oneShotCallBack:  blue LED off");
	BlueLed.Off();
}


void repeatCallBack( TimerHandle_t xTimer )
{
	static uint32_t counter = 0;

	SEGGER_SYSVIEW_PrintfHost("repeatCallBack:  toggle Green LED");
	// Toggle the green LED
	if(counter++ % 2)
	{
		GreenLed.On();
	}
	else
	{
		GreenLed.Off();
	}
}


