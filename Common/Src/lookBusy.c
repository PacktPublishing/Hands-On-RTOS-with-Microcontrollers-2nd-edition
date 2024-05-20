/*
 * Copyright (c) by PACKT, 2022, under the MIT License.
 - See the code-repository's license statement for more information:
 - https://github.com/PacktPublishing/Hands-On-RTOS-with-Microcontrollers-2nd-edition
*/

#include "lookBusy.h"
#include <stdint.h>
#include <stdbool.h>
#include <SEGGER_SYSVIEW.h>
#include "BSP_Init.h"

uint32_t itr_per_msec = 100;

void lookBusyInit();

/**
 * A simple function that keeps the CPU busy to consume time - simulating useful processing
 * Assumes that SysTick is configured for 1 msec for proper timing
 * 
 * NOTE: This function provides only only a rough approximation of time
 * @param ms_to_spin number of milliseconds lookBusy() will spin before returning
 */
void lookBusy( float ms_to_spin)
{
	static bool needsInit = true;
	if (needsInit)
	{
		needsInit = false;
		lookBusyInit();
	}

	uint32_t numIterations = itr_per_msec * ms_to_spin;
	uint32_t __attribute__((unused)) dontCare = 0;
	for(int i = 0; i < numIterations; i++)
	{
		dontCare = i % 4;
	}
}

/**
 * Assumes SysTick is configured for 1 msec tick rate
 * 
*/
void lookBusyInit()
{
	uint32_t lb_count = 0;
	itr_per_msec = 100;
	
	uint32_t starting_tick = HAL_GetTick();
	
	while(HAL_GetTick() < starting_tick+1);	//wait for the start of the next SysTick to get the full 1 millisecond tick

	//spin for 1 SysTick, counting how many times lookBusy was called
	while(HAL_GetTick() < starting_tick+2)
	{
		lookBusy(1);
		lb_count++;
	}

	//update how many individual for loop iterations of lookbusy there are per millisecond
	itr_per_msec =  lb_count * itr_per_msec; 
}
