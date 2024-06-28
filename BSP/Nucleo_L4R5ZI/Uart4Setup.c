/**
 * MIT License
 *
 * Copyright (c) 2024 Brian Amos
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

#include "UartQuickDirtyInit.h"
#include "Uart4Setup.h"
#include <stdint.h>
#include <string.h>
#include "stm32l4xx_hal.h"

// static const uint8_t uart4Msg[] = "0123456789abcdefghijklmnopqrstuvwxyz";
static const uint8_t uart4Msg[] = "data from uart4";
static DMA_HandleTypeDef uart4DmaTx;

static void uart4TxDmaStartRepeat(const uint8_t* Msg, uint16_t Len);
static void uart4TxDmaSetup(void);

/**
 * Setup UART4 to repeatedly transmit a message
 * via DMA (no CPU intervention required). This simulates
 * what would happen if there was data flowing from an
 * external off-chip source and let's us concentrate on
 * what's going on with UART2
 *
 * @param BaudRate desired baudrate for the UART4
 *
 * This is a quick and dirty setup...
 */
void SetupUart4ExternalSim(uint32_t BaudRate)
{
    // Setup DMA
    uart4TxDmaSetup();

    // GPIO pins are setup in BSP/Nucleo_L4R5ZI_Init
    STM_UartInit(UART4, BaudRate, &uart4DmaTx, NULL);

    // also enable DMA for UART4 Transmits
    UART4->CR3 |= USART_CR3_DMAT_Msk;

    /**
     * Start the repeating DMA transfer. Eventually, non-circular
     * receivers will lose a character here or there at high baud rates.
     */
    uart4TxDmaStartRepeat(uart4Msg, sizeof(uart4Msg));
}

static void uart4TxDmaSetup(void)
{
    __HAL_RCC_DMA1_CLK_ENABLE();
    __HAL_RCC_DMAMUX1_CLK_ENABLE();
    // No need to enable any interrupts
    HAL_NVIC_SetPriority(DMA1_Channel1_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(DMA1_Channel1_IRQn);

    // initialize the DMA peripheral to transfer uart4Msg
    // to UART4 repeatedly
    memset(&uart4DmaTx, 0, sizeof(uart4DmaTx));
    uart4DmaTx.Instance = DMA1_Channel1;
    uart4DmaTx.Init.Request = DMA_REQUEST_UART4_TX;
    uart4DmaTx.Init.Direction = DMA_MEMORY_TO_PERIPH; // Transfering out of memory and into the peripheral register
    uart4DmaTx.Init.PeriphInc = DMA_PINC_DISABLE; // Always keep the peripheral address the same (the Tx data register is always in the same location)
    uart4DmaTx.Init.MemInc = DMA_MINC_ENABLE; // Increment 1 byte at a time
    uart4DmaTx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    uart4DmaTx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    uart4DmaTx.Init.Mode = DMA_CIRCULAR; // This will automatically restart the transfer at the beginning after it has finished
    uart4DmaTx.Init.Priority = DMA_PRIORITY_VERY_HIGH; // We're setting high priority

    assert_param(HAL_DMA_Init(&uart4DmaTx) == HAL_OK);
    __HAL_DMA_DISABLE(&uart4DmaTx);

    // set the DMA transmit mode flag to enable DMA transfers
    UART4->CR3 |= USART_CR3_DMAT_Msk;
}

/**
 * starts a DMA transfer to the UART4 Tx register
 * that will automatically repeat after it is finished
 * @param Msg pointer to array to transfer
 * @param Len number of elements in the array
 */
static void uart4TxDmaStartRepeat(const uint8_t* Msg, uint16_t Len)
{
    // clear the transfer complete flag to make sure our transfer starts
    UART4->ICR |= USART_ICR_TCCF;
    assert_param(HAL_DMA_Start(&uart4DmaTx, (uint32_t)Msg, (uint32_t)&(UART4->TDR), Len) == HAL_OK);
}

/**
 * Start an interrupt-driven receive.
 */
void startReceiveInt()
{
    rxInProgress = true;
    USART2->CR3 |= USART_CR3_EIE; //enable error interrupts
    USART2->CR1 |= (USART_CR1_UE | USART_CR1_RXNEIE_RXFNEIE);
    //all 4 bits are for preemption priority -
    NVIC_SetPriority(USART2_IRQn, 6);
    NVIC_EnableIRQ(USART2_IRQn);
}