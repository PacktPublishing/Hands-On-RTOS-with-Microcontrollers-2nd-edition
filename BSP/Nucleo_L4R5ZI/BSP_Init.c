/*

   Changes for "Hands-On RTOS with Microcontrollers" 2nd Edition:
   * Fixed assert_failed()
   * Fixed StmRand()
 */

//#include <FreeRTOS.h>
#include "BSP_Init.h"
#include <BSP_GPIO.h>
#include <main.h>
#include <SEGGER_SYSVIEW.h>



// declarations for 'private' functions not exposed via header file
static void Error_Handler(void);
static void SystemClock_Config(void);
static void gpioPinsInit(void);
static void rngInit(void);

USBD_HandleTypeDef USBD_Device;

UART_HandleTypeDef huart4;
UART_HandleTypeDef uartInitStruct;
/************************************* PUBLIC FUNCTIONS **************************/

/**
 * Initialize the minimum amount of hardware that will be used in all examples
 * Other initialization might also be necessary (PWM for LED's, USART's, USB, etc)
 * and should be performed after the initial call to HWInit
 */
void HWInit( void )
{
	HAL_Init();
	SystemClock_Config();
	gpioPinsInit();			//initialize GPIO lines for LED's
	rngInit();
}

/**
 * NOTE:this function doesn't guarantee a new number every call
 * @param Min smallest number to generate
 * @param Max largest number to generate
 * @returns a pseudo random number from the dedicated RNG peripheral
 */
uint32_t StmRand( uint32_t Min, uint32_t Max )
{
    return ( (RNG->DR % ((Max-Min)+1)) + Min );
}

/**
  * @brief  System Clock Configuration
  *         The system Clock is configured as follow :
  *
  *         If define USB_USE_LSE_MSI_CLOCK enabled:
  *            System Clock source            = PLL (MSI)
  *            SYSCLK(Hz)                     = 80000000
  *            HCLK(Hz)                       = 80000000
  *            AHB Prescaler                  = 1
  *            APB1 Prescaler                 = 1
  *            APB2 Prescaler                 = 2
  *            MSI Frequency(Hz)              = 4800000
  *            LSE Frequency(Hz)              = 32768
  *            PLL_M                          = 6
  *            PLL_N                          = 40
  *            PLL_P                          = 7
  *            PLL_Q                          = 4
  *            PLL_R                          = 4
  *            Flash Latency(WS)              = 4
  *
  *         If define USB_USE_HSE_CLOCK enabled:
  *            System Clock source            = PLL (HSE)
  *            SYSCLK(Hz)                     = 80000000
  *            HCLK(Hz)                       = 80000000
  *            AHB Prescaler                  = 1
  *            APB1 Prescaler                 = 1
  *            APB2 Prescaler                 = 2
  *            HSE Frequency(Hz)              = 8000000
  *            PLL_M                          = 1
  *            PLL_N                          = 20
  *            PLL_P                          = 7
  *            PLL_Q                          = 4
  *            PLL_R                          = 2
  *            Flash Latency(WS)              = 4
  *
  * @param  None
  * @retval None
  */
static void SystemClock_Config(void)
{
  RCC_ClkInitTypeDef RCC_ClkInitStruct;
  RCC_OscInitTypeDef RCC_OscInitStruct;
  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct;

#if defined (USB_USE_LSE_MSI_CLOCK)

  /* Enable the LSE Oscillator */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSE;
  RCC_OscInitStruct.LSEState = RCC_LSE_ON;
  RCC_OscInitStruct.PLL.PLLState        = RCC_PLL_OFF;
  HAL_RCC_OscConfig(&RCC_OscInitStruct);

  /* Enable the CSS interrupt in case LSE signal is corrupted or not present */
  HAL_RCCEx_DisableLSECSS();

  /* Enable MSI Oscillator and activate PLL with MSI as source */
  RCC_OscInitStruct.OscillatorType      = RCC_OSCILLATORTYPE_MSI;
  RCC_OscInitStruct.MSIState            = RCC_MSI_ON;
  RCC_OscInitStruct.MSICalibrationValue = RCC_MSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.MSIClockRange       = RCC_MSIRANGE_11;
  RCC_OscInitStruct.PLL.PLLState        = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource       = RCC_PLLSOURCE_MSI;
  RCC_OscInitStruct.PLL.PLLM            = 6;
  RCC_OscInitStruct.PLL.PLLN            = 40;
  RCC_OscInitStruct.PLL.PLLP            = 7;
  RCC_OscInitStruct.PLL.PLLQ            = 4;
  RCC_OscInitStruct.PLL.PLLR            = 4;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
	  Error_Handler();
  }

  /* Enable MSI Auto-calibration through LSE */
  HAL_RCCEx_EnableMSIPLLMode();

  /* Select MSI output as USB clock source */
  PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_USB;
  PeriphClkInitStruct.UsbClockSelection = RCC_USBCLKSOURCE_MSI;
  HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct);

  /* Select PLL as system clock source and configure the HCLK, PCLK1 and PCLK2
  clocks dividers */
  RCC_ClkInitStruct.ClockType = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2);
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;
  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK)
  {
    Error_Handler();
  }

#elif defined (USB_USE_HSE_CLOCK)

  /* Enable HSE Oscillator and activate PLL with HSE as source */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 1;
  RCC_OscInitStruct.PLL.PLLN = 20;
  RCC_OscInitStruct.PLL.PLLR = 2;
  RCC_OscInitStruct.PLL.PLLP = 7;
  RCC_OscInitStruct.PLL.PLLQ = 4;

  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /* Select PLLSAI output as USB clock source */
  PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_USB;
  PeriphClkInitStruct.UsbClockSelection = RCC_USBCLKSOURCE_PLLSAI1;
  PeriphClkInitStruct.PLLSAI1.PLLSAI1N = 24;
  PeriphClkInitStruct.PLLSAI1.PLLSAI1Q = 4;
  PeriphClkInitStruct.PLLSAI1.PLLSAI1P = 1;
  PeriphClkInitStruct.PLLSAI1.PLLSAI1M = 1;
  PeriphClkInitStruct.PLLSAI1.PLLSAI1Source = RCC_PLLSOURCE_HSE;
  PeriphClkInitStruct.PLLSAI1.PLLSAI1ClockOut= RCC_PLLSAI1_48M2CLK;
  if(HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct)!= HAL_OK)
  {
    Error_Handler();
  }

  /* Select PLL as system clock source and configure the HCLK, PCLK1 and PCLK2
  clocks dividers */
  RCC_ClkInitStruct.ClockType = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2);
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV2;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;
  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK)
  {
    Error_Handler();
  }

#endif /* USB_USE_LSE_MSI_CLOCK */
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @param  None
  * @retval None
  */
static void Error_Handler(void)
{
  /* Turn LED3 on */
  BSP_LED_On(LED3);

  while (1)
  {
  }
}


/**
  * Initialize all relevant GPIO lines for LED's used in examples, as well as
  * USB pins
  */
static void gpioPinsInit(void)
{

// GPIO_InitTypeDef GPIO_InitStruct = {0};

 BSP_LED_Init(LED1);
 BSP_LED_Init(LED2);
 BSP_LED_Init(LED3);

  /* GPIO Ports Clock Enable */
 __HAL_RCC_GPIOH_CLK_ENABLE();
 __HAL_RCC_GPIOA_CLK_ENABLE();
 __HAL_RCC_GPIOB_CLK_ENABLE();
 __HAL_RCC_GPIOC_CLK_ENABLE();
 __HAL_RCC_GPIOD_CLK_ENABLE();
 __HAL_RCC_GPIOG_CLK_ENABLE();

 /*Configure GPIO pin Output Level */
// HAL_GPIO_WritePin(GPIOB, LD3_Pin|LD2_Pin, GPIO_PIN_RESET);

//  /*Configure GPIO pin Output Level */
//  HAL_GPIO_WritePin(USB_PowerSwitchOn_GPIO_Port, USB_PowerSwitchOn_Pin, GPIO_PIN_RESET);

 /*Configure GPIO pin : USER_Btn_Pin */
// GPIO_InitStruct.Pin = USER_Btn_Pin;
// GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
// GPIO_InitStruct.Pull = GPIO_NOPULL;
// HAL_GPIO_Init(USER_Btn_GPIO_Port, &GPIO_InitStruct);

 /*Configure GPIO pins : LD3_Pin LD2_Pin */
// GPIO_InitStruct.Pin = LD3_Pin|LD2_Pin|LD1_Pin;
// GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
// GPIO_InitStruct.Pull = GPIO_NOPULL;
// GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
// HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

//  /*Configure GPIO pin : USB_PowerSwitchOn_Pin */
//  GPIO_InitStruct.Pin = USB_PowerSwitchOn_Pin;
//  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
//  GPIO_InitStruct.Pull = GPIO_NOPULL;
//  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
//  HAL_GPIO_Init(USB_PowerSwitchOn_GPIO_Port, &GPIO_InitStruct);

//  /*Configure GPIO pin : USB_OverCurrent_Pin */
//  GPIO_InitStruct.Pin = USB_OverCurrent_Pin;
//  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
//  GPIO_InitStruct.Pull = GPIO_NOPULL;
//  HAL_GPIO_Init(USB_OverCurrent_GPIO_Port, &GPIO_InitStruct);

}

/**
 * Init the random number generator (RNG) peripheral
 */
static void rngInit( void )
{
	//start the peripheral clock
	__HAL_RCC_RNG_CLK_ENABLE();

	//enable the random number generator
	RNG->CR |= RNG_CR_RNGEN;
}

#ifdef  USE_FULL_ASSERT
/**
 * assert_failed() is a user-defined function.
 * It is called when assert_param() evaluates to true.
 * To use this function, put the following line in Inc\stm32f7xx_hal_conf.h:
 *     #define USE_FULL_ASSERT 1
 */
void assert_failed(uint8_t *file, uint32_t line)
{
	__disable_irq();
    BlueLed.On();
    RedLed.On();
    GreenLed.On();
    SEGGER_SYSVIEW_PrintfHost("assert failed. Line: %u.  File:", line);
    SEGGER_SYSVIEW_PrintfHost((char *) file);
    while(1);
}
#endif // USE_FULL_ASSERT
