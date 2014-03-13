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
    ISpi.Setup(DAC_SPI, boMSB, cpolIdleLow, cphaSecondEdge, sbFdiv2);
    ISpi.Enable();
}

void Dac_t::Set(uint16_t AData) {
    CskLo();
    ISpi.ReadWriteByte(0);
    ISpi.ReadWriteByte((AData>>8) & 0xFF);
    ISpi.ReadWriteByte(AData & 0xFF);
    CskHi();
}
