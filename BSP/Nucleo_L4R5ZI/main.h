/**
  ******************************************************************************
  * @file    USB_Device/CDC_Standalone/Inc/main.h 
  * @author  MCD Application Team
  * @brief   Header for main.c module
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2017 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
  
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

/* Includes ------------------------------------------------------------------*/
#include "usbd_core.h"
#include "usbd_desc.h"
#include "stm32l4xx_nucleo_144.h"
#include "usbd_cdc.h"
#include "usbd_cdc_interface.h"
/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/

/* Enable one of the following defines to select which clock will be used for USB */
#define USB_USE_LSE_MSI_CLOCK    /* Use MSI clock automatically trimmed by LSE as USB clock */
/* #define USB_USE_HSE_CLOCK  */ /* Use HSE as clock source for USB */

/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
void Toggle_Leds(void);

#endif /* __MAIN_H */

