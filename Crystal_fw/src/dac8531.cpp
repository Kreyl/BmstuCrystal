/*
 * dac8531.cpp

 *
 *  Created on: 12 марта 2014 г.
 *      Author: Roman Polovincev
 */
#include "dac8531.h"

Dac_t Dac;

void Dac_t::Init() {
    PinSetupOut(DAC_GPIO, DAC_CSK, omPushPull, pudNone);
    PinSetupAlterFunc(DAC_GPIO, DAC_CLK, omPushPull, pudNone, AF5);
    PinSetupAlterFunc(DAC_GPIO, DAC_MOSI, omPushPull, pudNone, AF5);
    CskHi();

    // ==== SPI ====    MSB first, master, ClkLowIdle, FirstEdge, Baudrate=f/2
    ISpi.Setup(DAC_SPI, boMSB, cpolIdleLow, cphaFirstEdge, sbFdiv8);
    ISpi.Enable();
}

uint16_t Dac_t::Measure() {
    CskLo();
    uint8_t b;
    uint32_t r = 0;
    b = ISpi.ReadWriteByte(30);
    CskHi();
}
