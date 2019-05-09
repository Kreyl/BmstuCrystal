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

// Analog switch
#define ADG_GPIO		GPIOC
#define ADG_IN1_PIN     0
#define ADG_IN2_PIN     1

// ADC
#define ADC_GPIO    	GPIOA
#define ADC_CLK     	5
#define ADC_MISO    	6
#define ADC_CSK     	4

// DAC
#define DAC_GPIO    	GPIOB
#define DAC_MOSI    	15
#define DAC_CSK     	12
#define DAC_CLK     	13

#define DAC_DMA         STM32_DMA2_STREAM3
#define DAC_DMA_CHNL    3

#endif

#if 1 // ========================= Timer =======================================
#define SAMPLING_TMR			TIM5
#define SAMPLING_TMR_IRQ		TIM5_IRQn
#define SAMPLING_TMR_IRQHandler	STM32_TIM5_HANDLER
#endif

#if 1 // =========================== SPI =======================================
// ADC
#define ADC_SPI     	SPI1

// DAC
#define DAC_SPI     	SPI2

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

// ADC
#define ADC_DMA         STM32_DMA2_STREAM0
#define ADC_DMA_CHNL    3

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
