# Hands-On-RTOS-with-Microcontrollers, 2nd Edition
Hands-On RTOS with Microcontrollers, 2nd Edition, from Packt Publishing
 
The second edition is in development, and it has not yet been published.

# About this Repository
Compared to example repository in the first edition of the book, the goal for 2E is to provide a framework that allows us to easily add additional hardware targets without any branching.  The underlying versions of FreeRTOS, SEGGER source, STM32F767 HAL, and middleware have been updated to the latest available (May 2024).

Build testing will be performed on both Windows 10 Pro and Ubuntu 20.04 LTS.  

PC Tools Used:
- STM32CubeIDE version 1.15.x
- SEGGER Ozone V 3.32a
- SEGGER SystemView v 3.52a

Source is based on the following:
- STM32F7_V1.17.1(HAL, ST Middleware)
- STM32F4_V1.18.0
- FreeRTOS 11.1.0
- SystemView Target Sources 3.54

# Supported Development Boards
At the time of publication, there will be support for two different development boards (Nucleo F767ZI and Nucleo L4R5ZI-P).

Because only ST-LINK V2 on-board debug probes can be reflashed to behave like a SEGGER J-Link, any newer dev boards added with an ST-LINK V3 will require an external J-Link debug probe.

| Dev Board         | ST_LINK Version | External J-Link required |
|-------------------|-----------------|--------------------------|
| Nucleo F767ZI     | V2              | NO                       |
| Nucleo L4R5ZI-P   | V2              | NO                       |
