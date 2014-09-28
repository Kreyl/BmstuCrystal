/*
 * dac8531.cpp

 *
 *  Created on: 12 марта 2014 г.
 *      Author: Roman Polovincev
 */
#include "dac8531.h"
#include "cmd_uart.h"

Dac_t Dac;

//extern "C" {
//// DMA irq
//void SIrqDmaHandler(void *p, uint32_t flags) { Dac.IrqDmaHandler_DAC(); }
//} // extern c

void Dac_t::Init() {
    PinSetupOut(DAC_GPIO, DAC_CSK, omPushPull, pudNone);
    PinSetupAlterFunc(DAC_GPIO, DAC_CLK, omPushPull, pudNone, AF5);
    PinSetupAlterFunc(DAC_GPIO, DAC_MOSI, omPushPull, pudNone, AF5);
    CskHi();

    // ==== SPI ====    MSB first, master, ClkLowIdle, FirstEdge, Baudrate=f/2
    ISpi.Setup(DAC_SPI, boMSB, cpolIdleLow, cphaSecondEdge, sbFdiv2);
    ISpi.Enable();
//    ISpi.SetModeTxOnly();
//    ISpi.EnableTxDma();


    // ==== DMA ====
//    dmaStreamAllocate     (DAC_DMA, IRQ_PRIO_MEDIUM, NULL, NULL);
//    dmaStreamSetPeripheral(DAC_DMA, &DAC_SPI->DR);
//    dmaStreamSetMode      (DAC_DMA, DAC_DMA_MODE);
}

void Dac_t::Set(uint16_t AData) {
    CskLo();
    ISpi.ReadWriteByte(0);
    ISpi.ReadWriteByte((AData>>8) & 0xFF);
    ISpi.ReadWriteByte(AData & 0xFF);
    CskHi();
}

void Dac_t::StartDMA_DAC() {
//    dmaStreamSetMemory0(DAC_DMA, &Rslt);
//    dmaStreamSetTransactionSize(DAC_DMA, 3);
//    dmaStreamSetMode(DAC_DMA, DAC_DMA_MODE);
//    dmaStreamEnable(DAC_DMA);
//    CskLo();
//    ISpi.Enable();
}
