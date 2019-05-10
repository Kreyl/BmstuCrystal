#pragma once

#include "kl_lib.h"
#include "ch.h"
#include "board.h"

class Dac_t {
private:
    Spi_t ISpi{DAC_SPI};
    void CskHi() { PinSetHi(DAC_CS); }
    void CskLo() { PinSetLo(DAC_CS); }
public:
    void Init();
    void Set(uint16_t AData);
};

extern Dac_t Dac;
