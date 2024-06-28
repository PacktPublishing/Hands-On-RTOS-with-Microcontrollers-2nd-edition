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
#include <task.h>
#include <queue.h>
#include <semphr.h>
#include <timers.h>
#include "BSP_Init.h"
#include <SEGGER_SYSVIEW.h>
#include <UartQuickDirtyInit.h>
#include "Uart4Setup.h"
#include <stdbool.h>
#include <string.h>

/*********************************************
 * A demonstration of a simple receive-only interrupt-driven UART driver,
 * that uses a queue.
 *********************************************/

#define STACK_SIZE 128

#define BAUDRATE 9600

void uartPrintOutTask( void* NotUsed);
void startUart4Traffic( TimerHandle_t xTimer );

static QueueHandle_t usart2_BytesReceived = NULL;

// Indicates USART2 is enabled to receive
volatile bool rxInProgress = false;

int main(void)
{
	BaseType_t retVal;
	HWInit();
	SEGGER_SYSVIEW_Conf();

	// Ensure proper priority grouping for freeRTOS
	NVIC_SetPriorityGrouping(0);

	// Setup a timer to kick off UART traffic (flowing out of UART4 TX line
	// and into USART2 RX line) 5 seconds after the scheduler starts.
	// The transmission needs to start after the receiver is ready for data.
	TimerHandle_t oneShotHandle =
	xTimerCreate(	"startUart4Traffic",
					5000 /portTICK_PERIOD_MS,
					pdFALSE,
					NULL,
					startUart4Traffic);
	assert_param(oneShotHandle != NULL);
	xTimerStart(oneShotHandle, 0);

    // Create the queue
	usart2_BytesReceived = xQueueCreate(10, sizeof(char));
	assert_param(usart2_BytesReceived != NULL);

    // Setup the task, making sure they have been properly created before moving on
	retVal = xTaskCreate(uartPrintOutTask, "uartPrint", STACK_SIZE, NULL, tskIDLE_PRIORITY + 3, NULL);
	assert_param(retVal == pdPASS);

	// Start the scheduler - shouldn't return unless there's a problem
	vTaskStartScheduler();

	// If you've wound up here, there is likely an issue with over-running the freeRTOS heap
	while(1)
	{
	}
}

void startUart4Traffic( TimerHandle_t xTimer )
{
	SetupUart4ExternalSim(BAUDRATE);
}

void uartPrintOutTask( void* NotUsed)
{
	char nextByte;
	STM_UartInit(USART2, BAUDRATE, NULL, NULL);
	startReceiveInt();

	while(1)
	{
		xQueueReceive(usart2_BytesReceived, &nextByte, portMAX_DELAY);
		// In "%c ", the space is a workaround for an apparent bug in SystemView.
		SEGGER_SYSVIEW_PrintfHost("%c ", nextByte);
	}
}

void USART2_IRQHandler( void )
{
	portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;
	SEGGER_SYSVIEW_RecordEnterISR();

	// First check for errors
	if(	USART2->ISR & (	USART_ISR_ORE_Msk |
						USART_ISR_NE_Msk |
						USART_ISR_FE_Msk |
						USART_ISR_PE_Msk ))
	{
		// Clear error flags
		USART2->ICR |= (USART_ICR_FECF |
						USART_ICR_PECF |
						USART_ICR_NCF |
						USART_ICR_ORECF);
	}

	if(	USART2->ISR & RXNE_MASK)
	{
		// Read the data register unconditionally to clear
		// the receive-not-empty interrupt if no reception is
		// in progress
		uint8_t tempVal = (uint8_t) USART2->RDR;

		if(rxInProgress)
		{
			xQueueSendFromISR(usart2_BytesReceived, &tempVal, &xHigherPriorityTaskWoken);
		}
	}
	SEGGER_SYSVIEW_RecordExitISR();
	portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}
