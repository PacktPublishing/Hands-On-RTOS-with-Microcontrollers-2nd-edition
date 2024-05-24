/*
 * Copyright (c) by PACKT, 2024, under the MIT License.
 - See the code-repository's license statement for more information:
 - https://github.com/PacktPublishing/Hands-On-RTOS-with-Microcontrollers-2nd-edition
*/
#ifndef __STM32_ASSERT_H
#define __STM32_ASSERT_H

#ifdef __cplusplus
extern "C" {
#endif

#ifdef  USE_FULL_ASSERT
#include "stdint.h"

#define assert_param(expr) ((expr) ? (void)0U : assert_failed((uint8_t *)__FILE__, __LINE__))
void assert_failed(uint8_t *file, uint32_t line);
#else
#define assert_param(expr) ((void)0U)
#endif /* USE_FULL_ASSERT */

#ifdef __cplusplus
}
#endif

#endif /* __STM32_ASSERT_H */

