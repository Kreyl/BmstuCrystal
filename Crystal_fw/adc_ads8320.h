/*
 * adc.h
 *
 *  Created on: 06 марта 2014 г.
 *      Author: g.kruglov
 */

#pragma once

#include "kl_lib.h"
#include "board.h"

#define ADC_DMA_MODE    STM32_DMA_CR_CHSEL(ADC_DMA_CHNL) | \
                        DMA_PRIORITY_MEDIUM | \
                        STM32_DMA_CR_MSIZE_BYTE | \
                        STM32_DMA_CR_PSIZE_BYTE | \
                        STM32_DMA_CR_MINC | \
                        STM32_DMA_CR_DIR_P2M |    /* Direction is peripheral to memory */ \
                        STM32_DMA_CR_TCIE         /* Enable Transmission Complete IRQ */

class Adc_t {
private:
    Spi_t ISpi{ADC_SPI};
    void CskHi() { PinSetHi(ADC_CS); }
    void CskLo() { PinSetLo(ADC_CS); }
    int32_t IRslt;
    const stm32_dma_stream_t *PDma;
public:
    int32_t Rslt;
    void Init();
    uint16_t Measure();
    void StartDMAMeasure();
    // Inner use
    void IrqDmaHandler();
};

extern Adc_t Adc;
