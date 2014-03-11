/*
 * adc_ads8320.cpp
 *
 *  Created on: 06 марта 2014 г.
 *      Author: g.kruglov
 */

#include "adc_ads8320.h"

Adc_t Adc;

void Adc_t::Init() {
    PinSetupOut(ADC_GPIO, ADC_CSK, omPushPull, pudNone);
    PinSetupAlterFunc(ADC_GPIO, ADC_CLK, omPushPull, pudNone, AF5);
    PinSetupAlterFunc(ADC_GPIO, ADC_MISO, omPushPull, pudNone, AF5);
    CskHi();
    // ==== SPI ====    MSB first, master, ClkLowIdle, FirstEdge, Baudrate=f/2
    ISpi.Setup(ADC_SPI, boMSB, cpolIdleLow, cphaFirstEdge, sbFdiv8);
    ISpi.Enable();
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
