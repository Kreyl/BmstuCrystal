#include "dac8531.h"
#include "uart.h"

Dac_t Dac;

void Dac_t::Init() {
    PinSetupOut(DAC_CS, omPushPull);
    PinSetupAlterFunc(DAC_CLK, omPushPull, pudNone, AF5);
    PinSetupAlterFunc(DAC_MOSI, omPushPull, pudNone, AF5);
    CskHi();

    // ==== SPI ====    MSB first, master, ClkLowIdle, FirstEdge, Baudrate=f/2 (SPI Freq supported up to 30 MHz)
    ISpi.Setup(boMSB, cpolIdleLow, cphaSecondEdge, 30000000);
    ISpi.Enable();
}

void Dac_t::Set(uint16_t AData) {
    CskLo();
    ISpi.ReadWriteByte(0);
    ISpi.ReadWriteByte((AData>>8) & 0xFF);
    ISpi.ReadWriteByte(AData & 0xFF);
    CskHi();
}
