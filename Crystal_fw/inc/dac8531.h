/*
 * dac8531.h
 *
 *  Created on: 12 марта 2014 г.
 *      Author: Roman Polovincev
 */

#ifndef DAC8531_H_
#define DAC8531_H_

#include "kl_lib_f40x.h"

#define DAC_SPI     SPI2
#define DAC_GPIO    GPIOB
#define DAC_MOSI    15
#define DAC_CSK     12
#define DAC_CLK     13

class Dac_t {
private:
    Spi_t ISpi;
    void CskHi() { PinSet(DAC_GPIO, DAC_CSK); }
    void CskLo() { PinClear(DAC_GPIO, DAC_CSK); }
public:
    void Init();
    void Set(uint16_t AData);
};

extern Dac_t Dac;

#endif /* DAC8531_H_ */
