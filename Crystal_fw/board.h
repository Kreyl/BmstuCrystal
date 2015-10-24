/*
 * board.h
 *
 *  Created on: 12 сент. 2015 г.
 *      Author: Kreyl
 */

#ifndef BOARD_H_
#define BOARD_H_

// General
#define CRYSTAL_FREQ_HZ     8000000    // Freq of external crystal

#if 1 // ========================== GPIO =======================================
// UART
#define UART_GPIO       GPIOA
#define UART_TX_PIN     9
#define UART_RX_PIN     10
#define UART_AF         AF7	// for all uarts

// LEDs
#define LED1_PIN        6
#define LED1_GPIO       GPIOC
#define LED2_PIN        7
#define LED2_GPIO       GPIOC
#define LED3_PIN        8
#define LED3_GPIO       GPIOC
#define LED_CNT			3
#endif

#if 1 // =========================== SPI =======================================
#define PCM_DATA_SPI_RccEnable() rccEnableSPI2(FALSE);

#endif

#if 1 // ========================== USART ======================================
#define UART            USART1
#define UART_TX_REG     UART->DR
#define UART_RX_REG     UART->DR
#endif

#if 1 // =========================== DMA =======================================
// Uart
#define UART_DMA_TX     STM32_DMA2_STREAM7
#define UART_DMA_RX     STM32_DMA2_STREAM5
#define UART_DMA_CHNL   4


#endif

#if 1 // ========================== USB ========================================
#define HAL_USE_SERIAL_USB			TRUE
/**
 * Serial over USB buffers size.
 * Configuration parameter, the buffer size must be a multiple of
 * the USB data endpoint maximum packet size.
 * The default is 64 bytes for both the transmission and receive buffers.
 */
#define SERIAL_USB_BUFFERS_SIZE     256

#endif


#endif /* BOARD_H_ */
