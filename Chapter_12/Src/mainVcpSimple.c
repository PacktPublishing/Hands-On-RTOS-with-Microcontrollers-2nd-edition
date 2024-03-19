/*

---------------------------------------------------------------------------------------

 Copyright for modifications to the original program:
 * Copyright (c) by PACKT, 2023, under the MIT License.
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
#include <usb_device.h>
#include <usbd_cdc_if.h>
#include <string.h>
#include <unsignedToAscii.h>

/*********************************************
 * A demonstration of improperly using CDC_Transmit_FS for sending data
 * over USB
 *********************************************/

#define STACK_SIZE 128

void sendDataTask( void* NotUsed);

int main(void)
{
    // Initialize hardware and HAL
    HWInit();

    // USB initialization
    MX_USB_DEVICE_Init();

    // Ensure proper priority grouping for FreeRTOS
    HAL_NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_4);

    // Setup the task that sends data, making sure it has been properly created before moving on
    assert_param(xTaskCreate(sendDataTask, "sendDataTask", STACK_SIZE, NULL, tskIDLE_PRIORITY + 2, NULL) == pdPASS);

    // Start the scheduler - shouldn't return unless there's a problem
    vTaskStartScheduler();

    // If you've wound up here, there is likely an issue with over-running the FreeRTOS heap
    while(1)
    {
    }
}

/**
 * Send data over USB, using CDC_Transmit_FS.
 *
 * CDC_Transmit_FS is non-blocking by default.
 * * It will not transmit if the USB transmit-process is busy,
 *   i.e., if the prior transmit has not completed
 */
void sendDataTask( void* NotUsed)
{
    #define STRING_BUFFER_SIZE 25
    char stringBuffer[STRING_BUFFER_SIZE];
    #define NUM_BUFFER_SIZE 11
    char numBuffer[NUM_BUFFER_SIZE];
    char * numPtr;
    uint32_t messageXcount = 0;
    uint32_t messageYcount = 0;
    USBD_CDC_HandleTypeDef *hcdc = NULL;

    // Ensure USB-initialization is completed.
    // * This code is implemented here, instead of main(), to take advantage of
    //   FreeRTOS's delay-function.
    while(hcdc == NULL)
    {
        // hUsbDeviceFS is a global handle, defined in Drivers/HandsOnRTOS/usb_device.c
        hcdc = (USBD_CDC_HandleTypeDef*)hUsbDeviceFS.pClassData;
        vTaskDelay(10);
    }

    while(1)
    {
        // Create the message to send:  message_X: <messageXcount>
        // * unsignedToAscii() is used instead of sprintf(),
        //   because the latter might not be thread-safe in FreeRTOS.
        numPtr = unsignedToAscii( ++messageXcount, (numBuffer+NUM_BUFFER_SIZE) );
        strcpy(stringBuffer, "message_X: ");
        strcpy(stringBuffer+strlen(stringBuffer), numPtr);
        strcpy(stringBuffer+strlen(stringBuffer), "\n");

        CDC_Transmit_FS((uint8_t*)stringBuffer, strlen(stringBuffer));

        // Create the message to send:  message_Y: <messageYcount>
        numPtr = unsignedToAscii( ++messageYcount, (numBuffer+NUM_BUFFER_SIZE) );
        strcpy(stringBuffer, "message_Y: ");
        strcpy(stringBuffer+strlen(stringBuffer), numPtr);
        strcpy(stringBuffer+strlen(stringBuffer), "\n");

        CDC_Transmit_FS((uint8_t*)stringBuffer, strlen(stringBuffer));

        vTaskDelay(2);
    }
}
