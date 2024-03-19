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
#include <Nucleo_F767ZI_GPIO.h>
#include <SEGGER_SYSVIEW.h>
#include <Nucleo_F767ZI_Init.h>
#include <stm32f7xx_hal.h>
#include <UartQuickDirtyInit.h>
#include "Uart4Setup.h"
#include <lookBusy.h>

/*********************************************
 * A demonstration of a polled UART driver for
 * sending and receiving
 *********************************************/

#define STACK_SIZE 128

void polledUartReceive ( void* NotUsed );
void uartPrintOutTask( void* NotUsed);
void startUpTask( void* NotUsed);

static QueueHandle_t uart2_BytesReceived = NULL;

uint32_t iterationsPerMilliSecond;

int main(void)
{
    HWInit();

    // Start UART4, and have it continuously send data.
    // UART4 continuously sends the string "data from uart4", including the null-terminator.
    SetupUart4ExternalSim(9600);

    SEGGER_SYSVIEW_Conf();
    HAL_NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_4); //ensure proper priority grouping for freeRTOS

    // Get the interation-rate for lookBusy()
    iterationsPerMilliSecond = lookBusyIterationRate();

    // Setup tasks, making sure they have been properly created before moving on
    assert_param(xTaskCreate(polledUartReceive, "polledUartRx", STACK_SIZE, NULL, tskIDLE_PRIORITY + 2, NULL) == pdPASS);
    assert_param(xTaskCreate(uartPrintOutTask, "uartPrintTask", STACK_SIZE, NULL, tskIDLE_PRIORITY + 3, NULL) == pdPASS);
    assert_param(xTaskCreate(startUpTask, "startUpTask", STACK_SIZE, NULL, tskIDLE_PRIORITY + 4, NULL) == pdPASS);

    // Create the queue.
    uart2_BytesReceived = xQueueCreate(10, sizeof(char));

    // Start the scheduler - shouldn't return unless there's a problem
    vTaskStartScheduler();

    // If you've wound up here, there is likely an issue with over-running the freeRTOS heap
    while(1)
    {
    }
}


// startUpTask() handles system start-up.
// * It is the highest-priority task.
// * It spins until the SystemView app is started in Record-mode.
// * Then, it deletes itself.
void startUpTask( void* NotUsed )
{
    // Indicate startUpTask has started
    BlueLed.On();

    // Spin until the user starts the SystemView app, in Record mode
    while(SEGGER_SYSVIEW_IsStarted()==0){
        lookBusy(iterationsPerMilliSecond);
    }

    BlueLed.Off();

    vTaskDelete(NULL);
}

// Gets queue-items and displays them on the SystemView app.
void uartPrintOutTask( void* NotUsed)
{
    char nextByte;

    while(1)
    {
        xQueueReceive(uart2_BytesReceived, &nextByte, portMAX_DELAY);

        // * The argument "%c " has an added space.
        // * The added space is a work-around to an apparent bug in SystemView.
        // * The bug: the argument "%c" results in no character being displayed
        //   in the SystemView app.
        SEGGER_SYSVIEW_PrintfHost("%c ", nextByte);
    }
}

/**
 * This receive task uses a queue to directly monitor
 * the USART2 peripheral.
 */
void polledUartReceive( void* NotUsed )
{
    uint8_t nextByte;
    // Setup USART2
    STM_UartInit(USART2, 9600, NULL, NULL);

    while(1)
    {
        while(!(USART2->ISR & USART_ISR_RXNE_Msk));
        nextByte = USART2->RDR;

        xQueueSend(uart2_BytesReceived, &nextByte, 0);
    }
}

