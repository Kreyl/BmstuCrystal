/*
    ChibiOS - Copyright (C) 2006..2015 Giovanni Di Sirio

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/

#ifndef _MCUCONF_H_
#define _MCUCONF_H_

/*
 * STM32F4xx drivers configuration.
 * The following settings override the default settings present in
 * the various device driver implementation headers.
 * Note that the settings for each driver only have effect if the whole
 * driver is enabled in halconf.h.
 *
 * IRQ priorities:
 * 15...0       Lowest...Highest.
 *
 * DMA priorities:
 * 0...3        Lowest...Highest.
 */

#define STM32F4xx_MCUCONF

// @KL
#define STM32_DMA_REQUIRED                  TRUE

/*
 * HAL driver system settings.
 */
#define STM32_NO_INIT                       FALSE
#define STM32_PVD_ENABLE                    FALSE
#define STM32_PLS                           STM32_PLS_LEV0
#define STM32_BKPRAM_ENABLE                 FALSE

/*
 * GPT driver system settings.
 */
#define STM32_GPT_USE_TIM1                  FALSE
#define STM32_GPT_USE_TIM2                  FALSE
#define STM32_GPT_USE_TIM3                  FALSE
#define STM32_GPT_USE_TIM4                  FALSE
#define STM32_GPT_USE_TIM5                  FALSE
#define STM32_GPT_USE_TIM6                  FALSE
#define STM32_GPT_USE_TIM7                  FALSE
#define STM32_GPT_USE_TIM8                  FALSE
#define STM32_GPT_USE_TIM9                  FALSE
#define STM32_GPT_USE_TIM11                 FALSE
#define STM32_GPT_USE_TIM12                 FALSE
#define STM32_GPT_USE_TIM14                 FALSE
#define STM32_GPT_TIM1_IRQ_PRIORITY         7
#define STM32_GPT_TIM2_IRQ_PRIORITY         7
#define STM32_GPT_TIM3_IRQ_PRIORITY         7
#define STM32_GPT_TIM4_IRQ_PRIORITY         7
#define STM32_GPT_TIM5_IRQ_PRIORITY         7
#define STM32_GPT_TIM6_IRQ_PRIORITY         7
#define STM32_GPT_TIM7_IRQ_PRIORITY         7
#define STM32_GPT_TIM8_IRQ_PRIORITY         7
#define STM32_GPT_TIM9_IRQ_PRIORITY         7
#define STM32_GPT_TIM11_IRQ_PRIORITY        7
#define STM32_GPT_TIM12_IRQ_PRIORITY        7
#define STM32_GPT_TIM14_IRQ_PRIORITY        7

/*
 * ICU driver system settings.
 */
#define STM32_ICU_USE_TIM1                  FALSE
#define STM32_ICU_USE_TIM2                  FALSE
#define STM32_ICU_USE_TIM3                  FALSE
#define STM32_ICU_USE_TIM4                  FALSE
#define STM32_ICU_USE_TIM5                  FALSE
#define STM32_ICU_USE_TIM8                  FALSE
#define STM32_ICU_USE_TIM9                  FALSE
#define STM32_ICU_TIM1_IRQ_PRIORITY         7
#define STM32_ICU_TIM2_IRQ_PRIORITY         7
#define STM32_ICU_TIM3_IRQ_PRIORITY         7
#define STM32_ICU_TIM4_IRQ_PRIORITY         7
#define STM32_ICU_TIM5_IRQ_PRIORITY         7
#define STM32_ICU_TIM8_IRQ_PRIORITY         7
#define STM32_ICU_TIM9_IRQ_PRIORITY         7

/*
 * SDC driver system settings.
 */
#define STM32_SDC_SDIO_DMA_PRIORITY         3
#define STM32_SDC_SDIO_IRQ_PRIORITY         9
#define STM32_SDC_WRITE_TIMEOUT_MS          250
#define STM32_SDC_READ_TIMEOUT_MS           25
#define STM32_SDC_CLOCK_ACTIVATION_DELAY    10
#define STM32_SDC_SDIO_UNALIGNED_SUPPORT    TRUE
#define STM32_SDC_SDIO_DMA_STREAM           STM32_DMA_STREAM_ID(2, 3)

/*
 * ST driver system settings.
 */
#define STM32_ST_IRQ_PRIORITY               8
#define STM32_ST_USE_TIMER                  2

/*
 * USB driver system settings.
 */
#define STM32_USB_USE_OTG1                  TRUE
#define STM32_USB_USE_OTG2                  FALSE
#define STM32_USB_OTG1_IRQ_PRIORITY         14
#define STM32_USB_OTG2_IRQ_PRIORITY         14
#define STM32_USB_OTG1_RX_FIFO_SIZE         512
#define STM32_USB_OTG2_RX_FIFO_SIZE         1024
#define STM32_USB_OTG_THREAD_PRIO           LOWPRIO
#define STM32_USB_OTG_THREAD_STACK_SIZE     128
#define STM32_USB_OTGFIFO_FILL_BASEPRI      0

#endif /* _MCUCONF_H_ */
