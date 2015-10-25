/*
 * adc_ads8320.cpp
 *
 *  Created on: 06 марта 2014 г.
 *      Author: g.kruglov
 */

#include "adc_ads8320.h"
#include "uart.h"
#include "core_cmInstr.h"
#include "main.h"

Adc_t Adc;

extern "C" {
// DMA irq
void SIrqDmaHandler(void *p, uint32_t flags) { Adc.IrqDmaHandler(); }
} // extern c

void Adc_t::Init() {
    PinSetupOut(ADC_GPIO, ADC_CSK, omPushPull, pudNone);
    PinSetupAlterFunc(ADC_GPIO, ADC_CLK, omPushPull, pudNone, AF5);
    PinSetupAlterFunc(ADC_GPIO, ADC_MISO, omPushPull, pudNone, AF5);
    CskHi();
    // ==== SPI ====    MSB first, master, ClkLowIdle, FirstEdge, Baudrate=...
    // Select baudrate (2.4MHz max): APB=32MHz => div = 16
    ISpi.Setup(ADC_SPI, boMSB, cpolIdleLow, cphaFirstEdge, sbFdiv16);
    ISpi.SetRxOnly();
    ISpi.EnableRxDma();
    // ==== DMA ====
    dmaStreamAllocate     (ADC_DMA, IRQ_PRIO_MEDIUM, SIrqDmaHandler, NULL);
    dmaStreamSetPeripheral(ADC_DMA, &ADC_SPI->DR);
    dmaStreamSetMode      (ADC_DMA, ADC_DMA_MODE);
}

uint16_t Adc_t::Measure() {
    CskLo();
    uint8_t b;
    uint32_t r = 0;
    b = ISpi.ReadWriteByte(0);
    r = b;
    b = ISpi.ReadWriteByte(0);
    r = (r << 8) | b;
    b = ISpi.ReadWriteByte(0);
    r = (r << 8) | b;
    CskHi();
    r >>= 2;
    r &= 0xFFFF;
    return r;
}

void Adc_t::StartDMAMeasure() {
    (void)ADC_SPI->DR;  // Clear input register
    dmaStreamSetMemory0(ADC_DMA, &Rslt);
    dmaStreamSetTransactionSize(ADC_DMA, 3);
    dmaStreamSetMode(ADC_DMA, ADC_DMA_MODE);
    dmaStreamEnable(ADC_DMA);
    CskLo();
    ISpi.Enable();
}

void Adc_t::IrqDmaHandler() {
    chSysLockFromISR();
    ISpi.Disable();
    CskHi();
    dmaStreamDisable(ADC_DMA);
    Rslt = __REV(Rslt);
    Rslt >>= 10;
    Rslt &= 0xFFFF;
//    Uart.Printf("%u\r", Rslt);
    App.SignalEvtI(EVTMSK_ADC_READY);
    chSysUnlockFromISR();
}
