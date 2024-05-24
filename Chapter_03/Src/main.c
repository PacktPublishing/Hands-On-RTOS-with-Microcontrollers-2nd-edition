 /* USER CODE BEGIN Header */

/**
 * TITLE:  Chapter 3 - Blinks an LED on the dev-board
 *
 * DESCRIPTION:
 * - This is an example program for the book
 *   "Hands On RTOS with Microcontrollers" (2nd edition), for chapter 3.
 * - This is a super-loop program. There is no OS.
 * - Code was added to toggle an LED
 * - For each loop iteration:
 *   - Toggles the green LED, with 1 second delay
 *   - Sends diagnostic messages to Segger SystemView
 *
 * USE:
 * - Intended for use with Segger's Ozone and SystemView
 *
 */

/**
   - Copyrighted (c) by PACKT, 2022, under the MIT License.
   - See the code-repository's license statement for more information:
     - https://github.com/PacktPublishing/Hands-On-RTOS-with-Microcontrollers-2nd-edition
 */

#include <BSP_Init.h>
#include <BSP_GPIO.h>
#include <SEGGER_SYSVIEW.h>
#include <stdint.h>

int main(void)
{
	uint32_t loopCounter = 0;

	HWInit();
	SEGGER_SYSVIEW_Conf();
	while (1)
	{

		// The loop-counter is just used for demonstrating SystemView's capabilities
		loopCounter++;

		// Write a message to the SystemView app
		SEGGER_SYSVIEW_PrintfHost("Starting loop-iteration: %u\n", loopCounter);

		GreenLed.On();
		// Wait for 1 second (1000 ms)
		HAL_Delay(1000);

		GreenLed.Off();
		HAL_Delay(1000);

		BlueLed.On();
		// Wait for 1 second (1000 ms)
		HAL_Delay(1000);

		BlueLed.Off();
		HAL_Delay(1000);

		RedLed.On();
		// Wait for 1 second (1000 ms)
		HAL_Delay(1000);

		RedLed.Off();
		HAL_Delay(1000);

		SEGGER_SYSVIEW_PrintfHost("Ending loop-iteration: %d\n", loopCounter);
	}
}
