/**
 * TITLE:  Chapter 5 - FreeRTOS with three tasks
 *
 * DESCRIPTION:
 * - This is an example program for the book
 *   "Hands On RTOS with Microcontrollers" (2nd edition), for chapter 6.
 * - FreeRTOS is run, with three tasks.  The tasks have different priorities.
 * - For each task, when it starts, it turns on an LED to indicate it started.
 * - The SystemView app is used to view the tasks' relative execution.
 *
 * USE:
 * - Intended for use with Segger's Ozone and SystemView
 * - The SystemView app's Record-mode must be run:
 *   - Record-mode must be started after (but not before) the dev-board's blue user-LED is on.
 *   - Task1's infinite loop won't run until Record-mode is started.
 *   - Task2 and Task3 won't run until Record-mode is started.
 */

/**
   Licenses:
   - Copyright (c) by PACKT, 2022, under the MIT License.
   - See the code-repository's license statement for more information:
     - https://github.com/PacktPublishing/Hands-On-RTOS-with-Microcontrollers-2nd-edition
 */

#include <FreeRTOS.h>
#include <BSP_Init.h>
#include <main.h>
#include <BSP_GPIO.h>
#include <task.h>
#include <SEGGER_SYSVIEW.h>
#include <lookBusy.h>

/**
 * 	Function prototypes
 */
void Task1(void *argument);
void Task2(void *argument);
void Task3(void *argument);

int main(void)
{
    // Recommended minimum stack size per task
    //   128 * 4 = 512 bytes
    const static uint32_t stackSize = 128;
    HWInit();
    SEGGER_SYSVIEW_Conf();
    HAL_NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_4); // ensure proper priority grouping for freeRTOS

    if (xTaskCreate(Task1, "task1", stackSize, NULL, tskIDLE_PRIORITY + 3, NULL) == pdPASS)
    {
        if (xTaskCreate(Task2, "task2", stackSize, NULL, tskIDLE_PRIORITY + 2, NULL) == pdPASS)
        {
            if (xTaskCreate(Task3, "task3", stackSize, NULL, tskIDLE_PRIORITY + 1, NULL) == pdPASS)
            {
                // Start the scheduler - shouldn't return unless there's a problem
                vTaskStartScheduler();
            }
        }
    }

    // If you've wound up here, there is likely an issue with over-running the FreeRTOS heap
    while (1)
    {
    }
}

void Task1(void *argument)
{
    uint32_t iterationCount = 0;
    BlueLed.On();

    // * The SystemView app's Record-mode cannot be started until after the FreeRTOS scheduler is started.
    //   This is a SystemView requirement.
    // * This while-loop spins until Record-mode is started.
    // * Also, spinning until Record-mode is started ensures all of the SEGGER_SYSVIEW_PrintfHost()
    //   messages will be displayed.
    // * lookBusy() is used to slow-down the while-loop.
    //   * SEGGER_SYSVIEW_IsStarted() communicates with the SystemView app on the development computer.
    //   * There is a non-trivial communication latency. If SEGGER_SYSVIEW_IsStarted() is called
    //     too frequently, it can cause overflow in SystemView.
    while (SEGGER_SYSVIEW_IsStarted() == 0)
    {
        lookBusy(100);
    }
    SEGGER_SYSVIEW_PrintfHost("Task1: starting\n");

    while (1)
    {
        // SEGGER_SYSVIEW_PrintfHost is only called every 100 iterations, to prevent SystemView overflow.
        iterationCount++;
        if ((iterationCount % 100) == 1)
        {
            SEGGER_SYSVIEW_PrintfHost("Task1. Iteration: %u\n", iterationCount);
        }
        // Simulate 250 usec of useful processing (250 usec of cpu time)
        lookBusy(0.250);
        // vTaskDelay causes the task to be delayed for the specified number of SysTicks, i.e., 5
        // pdMS_TO_TICKS converts millisecond to SysTicks
        vTaskDelay(pdMS_TO_TICKS(5));
    }
}

void Task2(void *argument)
{
    uint32_t iterationCount = 0;
    GreenLed.On();
    SEGGER_SYSVIEW_PrintfHost("Task2: starting\n");

    while (1)
    {
        iterationCount++;
        if ((iterationCount % 100) == 1)
        {
            SEGGER_SYSVIEW_PrintfHost("Task2. Iteration: %u\n", iterationCount);
        }
        // Simulate useful processing. Spin for 500 usec (processor time).
        lookBusy(0.500);
        // Delay processing until the next SysTick.
        // At the next SysTick:
        // * If Task1 is being delayed, then Task2 will resume.
        // * If Task1 is not being delayed, then Task1 will run.
        vTaskDelay(pdMS_TO_TICKS(1));
    }
}

void Task3(void *argument)
{
    uint32_t iterationCount = 0;

    RedLed.On();
    SEGGER_SYSVIEW_PrintfHost("Task3: starting\n");
    // For Task3, lookBusy() needs to spin for 2 ms (processor time).

    while (1)
    {
        iterationCount++;
        if ((iterationCount % 100) == 1)
        {
            SEGGER_SYSVIEW_PrintfHost("Task3. Iteration: %u\n", iterationCount);
        }
        // Simulate useful processing. Spin for 2 ms  (processor time).
        lookBusy(2);
    }
}
