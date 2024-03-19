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
#include <Nucleo_F767ZI_GPIO.h>
#include <SEGGER_SYSVIEW.h>
#include <Nucleo_F767ZI_Init.h>
#include <stm32f7xx_hal.h>

/*********************************************
 * A simple demonstration of using task
 * notifications in place of a queue
 *********************************************/

#define STACK_SIZE 128

#define RED_LED_MASK   0x00000001
#define BLUE_LED_MASK  0x00000002
#define GREEN_LED_MASK 0x00000004

void recvTask( void* NotUsed );
void sendingTask( void* NotUsed );

// This task handle will be used to send
// task notifications to recvTask
static TaskHandle_t recvTaskHandle = NULL;

int main(void)
{
	HWInit();
	SEGGER_SYSVIEW_Conf();
	HAL_NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_4);	// Ensure proper priority grouping for freeRTOS

	// Setup tasks, making sure they have been properly created before moving on
	assert_param(xTaskCreate(recvTask, "recvTask", STACK_SIZE, NULL, tskIDLE_PRIORITY + 2, &recvTaskHandle) == pdPASS);
	assert_param(xTaskCreate(sendingTask, "sendingTask", STACK_SIZE, NULL, tskIDLE_PRIORITY + 1, NULL) == pdPASS);

	assert_param(recvTaskHandle != NULL);

	// Start the scheduler - shouldn't return unless there's a problem
	vTaskStartScheduler();

	// If you've wound up here, there is likely an issue with over-running the FreeRTOS heap
	while(1)
	{
	}
}

/**
 * recvTask waits for a new task notification
 */
void recvTask( void* NotUsed )
{
    uint32_t notificationvalue;

	while(1)
	{
		// Wait for the next notification value
	    xTaskNotifyWait(0, 0, &notificationvalue, portMAX_DELAY);

		if((notificationvalue & RED_LED_MASK) != 0)
			RedLed.On();
		else
			RedLed.Off();

		if((notificationvalue & BLUE_LED_MASK) != 0)
			BlueLed.On();
		else
			BlueLed.Off();

		if((notificationvalue & GREEN_LED_MASK) != 0)
			GreenLed.On();
		else
			GreenLed.Off();
	}
}

/**
 * sendingTask sends task notifications
 *
 */
void sendingTask( void* NotUsed )
{
	while(1)
	{
		// Send the notification to recvTask, to turn on the Red LED, and turn the other LEDs off.
		// Since we're using eSetValueWithOverwrite, there is no need to check the return value.
		xTaskNotify( recvTaskHandle, RED_LED_MASK, eSetValueWithOverwrite);
		vTaskDelay(500);

		xTaskNotify( recvTaskHandle, BLUE_LED_MASK, eSetValueWithOverwrite);
		vTaskDelay(500);

		xTaskNotify( recvTaskHandle, GREEN_LED_MASK, eSetValueWithOverwrite);
		vTaskDelay(500);

		xTaskNotify( recvTaskHandle, RED_LED_MASK | BLUE_LED_MASK | GREEN_LED_MASK, eSetValueWithOverwrite);
		vTaskDelay(500);

		xTaskNotify( recvTaskHandle, 0, eSetValueWithOverwrite);
		vTaskDelay(500);
	}
}

