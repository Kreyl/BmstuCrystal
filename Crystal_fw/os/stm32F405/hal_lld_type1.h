/*
    ChibiOS - Copyright (C) 2006..2018 Giovanni Di Sirio

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

/**
 * @file    STM32F4xx/hal_lld_type1.h
 * @brief   STM32F4xx/STM32F2xx HAL subsystem low level driver header.
 * @pre     This module requires the following macros to be defined in the
 *          @p board.h file:
 *          - STM32_LSECLK.
 *          - STM32_LSE_BYPASS (optionally).
 *          - STM32_HSECLK.
 *          - STM32_HSE_BYPASS (optionally).
 *          - STM32_VDD (as hundredths of Volt).
 *          .
 *          One of the following macros must also be defined:
 *          - STM32F2XX for High-performance STM32F2 devices.
 *          - STM32F405xx, STM32F415xx, STM32F407xx, STM32F417xx,
 *            STM32F446xx for High-performance STM32F4 devices of
 *            Foundation line.
 *          - STM32F401xx, STM32F410xx, STM32F411xx, STM32F412xx
 *            for High-performance STM32F4 devices of Access line.
 *          - STM32F427xx, STM32F437xx, STM32F429xx, STM32F439xx, STM32F469xx,
 *            STM32F479xx for High-performance STM32F4 devices of Advanced line.
 *          .
 *
 * @addtogroup HAL
 * @{
 */

#ifndef HAL_LLD_TYPE1_H
#define HAL_LLD_TYPE1_H

/*===========================================================================*/
/* Driver constants.                                                         */
/*===========================================================================*/

/**
 * @brief   Defines the support for realtime counters in the HAL.
 */
#define HAL_IMPLEMENTS_COUNTERS              TRUE

/**
 * @name    Platform identification macros
 * @{
 */
#if defined(STM32F205xx) || defined(__DOXYGEN__)
#define PLATFORM_NAME           "STM32F205 High Performance"

#elif defined(STM32F207xx)
#define PLATFORM_NAME           "STM32F207 High Performance"

#elif defined(STM32F215xx)
#define PLATFORM_NAME           "STM32F215 High Performance"

#elif defined(STM32F217xx)
#define PLATFORM_NAME           "STM32F217 High Performance"

#elif defined(STM32F401xx)
#define PLATFORM_NAME           "STM32F401 High Performance with DSP and FPU"

#elif defined(STM32F405xx)
#define PLATFORM_NAME           "STM32F405 High Performance with DSP and FPU"

#elif defined(STM32F407xx)
#define PLATFORM_NAME           "STM32F407 High Performance with DSP and FPU"

#elif defined(STM32F410xx)
#define PLATFORM_NAME           "STM32F410 High Performance with DSP and FPU"

#elif defined(STM32F411xx)
#define PLATFORM_NAME           "STM32F411 High Performance with DSP and FPU"

#elif defined(STM32F412xx)
#define PLATFORM_NAME           "STM32F412 High Performance with DSP and FPU"

#elif defined(STM32F415xx)
#define PLATFORM_NAME           "STM32F415 High Performance with DSP and FPU"

#elif defined(STM32F417xx)
#define PLATFORM_NAME           "STM32F417 High Performance with DSP and FPU"

#elif defined(STM32F427xx)
#define PLATFORM_NAME           "STM32F427 High Performance with DSP and FPU"

#elif defined(STM32F429xx)
#define PLATFORM_NAME           "STM32F429 High Performance with DSP and FPU"

#elif defined(STM32F437xx)
#define PLATFORM_NAME           "STM32F437 High Performance with DSP and FPU"

#elif defined(STM32F439xx)
#define PLATFORM_NAME           "STM32F439 High Performance with DSP and FPU"

#elif defined(STM32F446xx)
#define PLATFORM_NAME           "STM32F446 High Performance with DSP and FPU"

#elif defined(STM32F469xx)
#define PLATFORM_NAME           "STM32F469 High Performance with DSP and FPU"

#elif defined(STM32F479xx)
#define PLATFORM_NAME           "STM32F479 High Performance with DSP and FPU"

#else
#error "STM32F2xx/F4xx device not specified"
#endif
/** @} */

/**
 * @name    Absolute Maximum Ratings
 * @{
 */
#if defined(STM32F427xx) || defined(STM32F437xx) ||                         \
    defined(STM32F429xx) || defined(STM32F439xx) ||                         \
    defined(STM32F469xx) || defined(STM32F479xx) || defined(__DOXYGEN__)
/**
 * @brief   Absolute maximum system clock.
 */
#define STM32_SYSCLK_MAX        180000000

/**
 * @brief   Maximum HSE clock frequency.
 */
#define STM32_HSECLK_MAX        26000000

/**
 * @brief   Maximum HSE clock frequency using an external source.
 */
#define STM32_HSECLK_BYP_MAX    50000000

/**
 * @brief   Minimum HSE clock frequency.
 */
#define STM32_HSECLK_MIN        4000000

/**
 * @brief   Minimum HSE clock frequency using an external source.
 */
#define STM32_HSECLK_BYP_MIN    1000000

/**
 * @brief   Maximum LSE clock frequency.
 */
#define STM32_LSECLK_MAX        32768

/**
 * @brief   Maximum LSE clock frequency using an external source.
 */
#define STM32_LSECLK_BYP_MAX    1000000

/**
 * @brief   Minimum LSE clock frequency.
 */
#define STM32_LSECLK_MIN        32768

/**
 * @brief   Maximum PLLs input clock frequency.
 */
#define STM32_PLLIN_MAX         2100000

/**
 * @brief   Minimum PLLs input clock frequency.
 */
#define STM32_PLLIN_MIN         950000

/**
 * @brief   Maximum PLLs VCO clock frequency.
 */
#define STM32_PLLVCO_MAX        432000000

/**
 * @brief   Minimum PLLs VCO clock frequency.
 */
#define STM32_PLLVCO_MIN        192000000

/**
 * @brief   Maximum PLL output clock frequency.
 */
#define STM32_PLLOUT_MAX        180000000

/**
 * @brief   Minimum PLL output clock frequency.
 */
#define STM32_PLLOUT_MIN        24000000

/**
 * @brief   Maximum PLLI2S output clock frequency.
 */
#define STM32_PLLI2SOUT_MAX     216000000

/**
 * @brief   Maximum PLLSAI output clock frequency.
 */
#define STM32_PLLSAIOUT_MAX     216000000

/**
 * @brief   Maximum APB1 clock frequency.
 */
#define STM32_PCLK1_MAX         (STM32_PLLOUT_MAX / 4)

/**
 * @brief   Maximum APB2 clock frequency.
 */
#define STM32_PCLK2_MAX         (STM32_PLLOUT_MAX / 2)

/**
 * @brief   Maximum SPI/I2S clock frequency.
 */
#define STM32_SPII2S_MAX        45000000
#endif

#if defined(STM32F40_41xxx)
#define STM32_SYSCLK_MAX        168000000
#define STM32_HSECLK_MAX        26000000
#define STM32_HSECLK_BYP_MAX    50000000
#define STM32_HSECLK_MIN        4000000
#define STM32_HSECLK_BYP_MIN    1000000
#define STM32_LSECLK_MAX        32768
#define STM32_LSECLK_BYP_MAX    1000000
#define STM32_LSECLK_MIN        32768
#define STM32_PLLIN_MAX         2100000
#define STM32_PLLIN_MIN         950000
#define STM32_PLLVCO_MAX        432000000
#define STM32_PLLVCO_MIN        192000000
#define STM32_PLLOUT_MAX        168000000
#define STM32_PLLOUT_MIN        24000000
#define STM32_PCLK1_MAX         42000000
#define STM32_PCLK2_MAX         84000000
#define STM32_SPII2S_MAX        42000000
#endif

#if defined(STM32F401xx)
#define STM32_SYSCLK_MAX        84000000
#define STM32_HSECLK_MAX        26000000
#define STM32_HSECLK_BYP_MAX    50000000
#define STM32_HSECLK_MIN        4000000
#define STM32_HSECLK_BYP_MIN    1000000
#define STM32_LSECLK_MAX        32768
#define STM32_LSECLK_BYP_MAX    1000000
#define STM32_LSECLK_MIN        32768
#define STM32_PLLIN_MAX         2100000
#define STM32_PLLIN_MIN         950000
#define STM32_PLLVCO_MAX        432000000
#define STM32_PLLVCO_MIN        192000000
#define STM32_PLLOUT_MAX        84000000
#define STM32_PLLOUT_MIN        24000000
#define STM32_PCLK1_MAX         42000000
#define STM32_PCLK2_MAX         84000000
#define STM32_SPII2S_MAX        42000000
#endif

#if defined(STM32F410xx) || defined(STM32F411xx) ||                         \
    defined(STM32F412xx)
#define STM32_SYSCLK_MAX        100000000
#define STM32_HSECLK_MAX        26000000
#define STM32_HSECLK_BYP_MAX    50000000
#define STM32_HSECLK_MIN        4000000
#define STM32_HSECLK_BYP_MIN    1000000
#define STM32_LSECLK_MAX        32768
#define STM32_LSECLK_BYP_MAX    1000000
#define STM32_LSECLK_MIN        32768
#define STM32_PLLIN_MAX         2100000
#define STM32_PLLIN_MIN         950000
#define STM32_PLLVCO_MAX        432000000
#define STM32_PLLVCO_MIN        100000000
#define STM32_PLLOUT_MAX        100000000
#define STM32_PLLOUT_MIN        24000000
#define STM32_PCLK1_MAX         50000000
#define STM32_PCLK2_MAX         100000000
#define STM32_SPII2S_MAX        50000000
#endif

#if defined(STM32F446xx)
#define STM32_SYSCLK_MAX        180000000
#define STM32_HSECLK_MAX        26000000
#define STM32_HSECLK_BYP_MAX    50000000
#define STM32_HSECLK_MIN        4000000
#define STM32_HSECLK_BYP_MIN    1000000
#define STM32_LSECLK_MAX        32768
#define STM32_LSECLK_BYP_MAX    1000000
#define STM32_LSECLK_MIN        32768
#define STM32_PLLIN_MAX         2100000
#define STM32_PLLIN_MIN         950000
#define STM32_PLLVCO_MAX        432000000
#define STM32_PLLVCO_MIN        100000000
#define STM32_PLLOUT_MAX        180000000
#define STM32_PLLOUT_MIN        12500000
#define STM32_PLLI2SOUT_MAX     216000000
#define STM32_PLLSAIOUT_MAX     216000000
#define STM32_PCLK1_MAX         (STM32_PLLOUT_MAX / 4)
#define STM32_PCLK2_MAX         (STM32_PLLOUT_MAX / 2)
#define STM32_SPII2S_MAX        45000000
#endif

#if defined(STM32F2XX)
#define STM32_SYSCLK_MAX        120000000
#define STM32_HSECLK_MAX        26000000
#define STM32_HSECLK_BYP_MAX    26000000
#define STM32_HSECLK_MIN        1000000
#define STM32_HSECLK_BYP_MIN    1000000
#define STM32_LSECLK_MAX        32768
#define STM32_LSECLK_BYP_MAX    1000000
#define STM32_LSECLK_MIN        32768
#define STM32_PLLIN_MAX         2000000
#define STM32_PLLIN_MIN         950000
#define STM32_PLLVCO_MAX        432000000
#define STM32_PLLVCO_MIN        192000000
#define STM32_PLLOUT_MAX        120000000
#define STM32_PLLOUT_MIN        24000000
#define STM32_PCLK1_MAX         30000000
#define STM32_PCLK2_MAX         60000000
#define STM32_SPII2S_MAX        30000000
#endif
/** @} */

/*===========================================================================*/
/* Derived constants and error checks.                                       */
/*===========================================================================*/

#if defined(STM32F4XX) || defined(__DOXYGEN__)
/*
 * Configuration-related checks.
 */
#if !defined(STM32F4xx_MCUCONF)
#error "Using a wrong mcuconf.h file, STM32F4xx_MCUCONF not defined"
#endif

#else /* !defined(STM32F4XX) */
/*
 * Configuration-related checks.
 */
#if !defined(STM32F2xx_MCUCONF)
#error "Using a wrong mcuconf.h file, STM32F2xx_MCUCONF not defined"
#endif
#endif /* !defined(STM32F4XX) */



#ifdef __cplusplus
extern "C" {
#endif
  void hal_lld_init(void);
#ifdef __cplusplus
}
#endif

#endif /* HAL_LLD_TYPE1_H */

/** @} */
