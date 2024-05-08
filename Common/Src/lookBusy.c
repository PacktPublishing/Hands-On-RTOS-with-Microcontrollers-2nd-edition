/*
 * Copyright (c) by PACKT, 2022, under the MIT License.
 - See the code-repository's license statement for more information:
 - https://github.com/PacktPublishing/Hands-On-RTOS-with-Microcontrollers-2nd-edition
*/

#include "lookBusy.h"
#include <stdint.h>
#include <SEGGER_SYSVIEW.h>

/**
 * A simple function that keeps the CPU busy to consume time - simulating useful processing
 */
void lookBusy( void )
{
	volatile uint32_t __attribute__((unused)) dontCare = 0;
	for(int i = 0; i < 50E3; i++)
	{
		dontCare = i % 4;
	}
	SEGGER_SYSVIEW_PrintfHost("looking busy\n");
}
