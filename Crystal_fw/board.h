#pragma once

// ==== General ====
#define APP_NAME            "BmStuCrystal"
// Version of PCB
#define PCB_VER             201904

// MCU type as defined in the ST header.
#define STM32F405xx

// Freq of external crystal if any. Leave it here even if not used.
#define CRYSTAL_FREQ_HZ         12000000

// OS timer settings
#define STM32_ST_IRQ_PRIORITY   2
#define STM32_ST_USE_TIMER      2

#define STM32_DMA_REQUIRED      TRUE    // Leave this macro name for OS

#if 1 // ========================== GPIO =======================================
// EXTI
#define INDIVIDUAL_EXTI_IRQ_REQUIRED    FALSE
// UART
#define UART_GPIO       GPIOA
#define UART_TX_PIN     9
#define UART_RX_PIN     10

#define DEBUG_PIN       GPIOA, 8

// LEDs
#define LED1_PIN        { GPIOB, 6, TIM4, 1, invNotInverted, omPushPull, 255 }
#define LED2_PIN        { GPIOB, 7, TIM4, 2, invNotInverted, omPushPull, 255 }
#define LED3_PIN        { GPIOB, 8, TIM4, 3, invNotInverted, omPushPull, 255 }

// ADC
#define ADC_CLK     	GPIOA, 5
#define ADC_MISO    	GPIOA, 6
#define ADC_CS          GPIOA, 7

// DAC
#define DAC_MOSI    	GPIOB, 15
#define DAC_CS      	GPIOB, 12
#define DAC_CLK     	GPIOB, 13

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

#if 1 // =========================== DMA =======================================
// ==== Uart ====
// Remap is made automatically if required
#define UART_DMA_TX             STM32_DMA_STREAM_ID(2, 7)
#define UART_DMA_RX             STM32_DMA_STREAM_ID(2, 5)
#define UART_DMA_CHNL           4
#define UART_DMA_TX_MODE(Chnl)  (STM32_DMA_CR_CHSEL(Chnl) | DMA_PRIORITY_LOW | STM32_DMA_CR_MSIZE_BYTE | STM32_DMA_CR_PSIZE_BYTE | STM32_DMA_CR_MINC | STM32_DMA_CR_DIR_M2P | STM32_DMA_CR_TCIE)
#define UART_DMA_RX_MODE(Chnl)  (STM32_DMA_CR_CHSEL(Chnl) | DMA_PRIORITY_MEDIUM | STM32_DMA_CR_MSIZE_BYTE | STM32_DMA_CR_PSIZE_BYTE | STM32_DMA_CR_MINC | STM32_DMA_CR_DIR_P2M | STM32_DMA_CR_CIRC)


#define DAC_DMA         STM32_DMA_STREAM_ID(2, 3)
#define DAC_DMA_CHNL    3

// ADC
#define ADC_DMA         STM32_DMA_STREAM_ID(2, 0)
#define ADC_DMA_CHNL    3

#endif

#if 1 // ========================== USART ======================================
#define PRINTF_FLOAT_EN TRUE
#define UART_TXBUF_SZ   4096
#define UART_RXBUF_SZ   99

#define UARTS_CNT       1

#define CMD_UART_PARAMS \
    USART1, UART_GPIO, UART_TX_PIN, UART_GPIO, UART_RX_PIN, \
    UART_DMA_TX, UART_DMA_RX, UART_DMA_TX_MODE(UART_DMA_CHNL), UART_DMA_RX_MODE(UART_DMA_CHNL)

#endif

#if 1 // ========================== USB ========================================
#define HAL_USE_USB                 TRUE
#define HAL_USE_SERIAL_USB			TRUE
/**
 * Serial over USB buffers size.
 * Configuration parameter, the buffer size must be a multiple of
 * the USB data endpoint maximum packet size.
 * The default is 64 bytes for both the transmission and receive buffers.
 */
#define SERIAL_USB_BUFFERS_SIZE     256

#endif

