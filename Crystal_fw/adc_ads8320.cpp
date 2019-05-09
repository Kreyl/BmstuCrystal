/*
 * adc_ads8320.cpp
 *
 *  Created on: 06 марта 2014 г.
 *      Author: g.kruglov
 */

#include "adc_ads8320.h"
#include "uart.h"
#include "main.h"
#include "MsgQ.h"

Adc_t Adc;

// DMA irq
extern "C"
void SIrqDmaHandler(void *p, uint32_t flags) { Adc.IrqDmaHandler(); }

void Adc_t::Init() {
    PinSetupOut(ADC_CS, omPushPull);
    PinSetupAlterFunc(ADC_CLK, omPushPull, pudNone, AF5);
    PinSetupAlterFunc(ADC_MISO, omPushPull, pudNone, AF5);
    CskHi();
    // ==== SPI ====    MSB first, master, ClkLowIdle, FirstEdge, Baudrate=...
    // Select baudrate (2.4MHz max): APB=32MHz => div = 16
    ISpi.Setup(boMSB, cpolIdleLow, cphaFirstEdge, 2400000);
    ISpi.SetRxOnly();
    ISpi.EnableRxDma();
    // ==== DMA ====
    PDma = dmaStreamAlloc(ADC_DMA, IRQ_PRIO_MEDIUM, SIrqDmaHandler, nullptr);
    dmaStreamSetPeripheral(PDma, &ADC_SPI->DR);
    dmaStreamSetMode      (PDma, ADC_DMA_MODE);
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
    dmaStreamSetMemory0(PDma, &IRslt);
    dmaStreamSetTransactionSize(PDma, 3);
    dmaStreamSetMode(PDma, ADC_DMA_MODE);
    dmaStreamEnable(PDma);
    CskLo();
    ISpi.Enable();
}

void Adc_t::IrqDmaHandler() {
    chSysLockFromISR();
    ISpi.Disable();
    CskHi();
    dmaStreamDisable(PDma);
    IRslt = __REV(IRslt);
    IRslt >>= 10;
    IRslt &= 0xFFFF;
    Rslt = IRslt;
//    Uart.Printf("%u\r", Rslt);
    EvtQMain.SendNowOrExitI(EvtMsg_t(evtIdAdcReady));
    chSysUnlockFromISR();
}
