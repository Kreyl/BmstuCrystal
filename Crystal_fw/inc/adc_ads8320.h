/*
 * adc.h
 *
 *  Created on: 06 марта 2014 г.
 *      Author: g.kruglov
 */

#ifndef ADC_H_
#define ADC_H_

#include "kl_lib_f40x.h"

#define ADC_SPI     SPI1
#define ADC_GPIO    GPIOA
#define ADC_CLK     5
#define ADC_MISO    6
#define ADC_CSK     4

class Adc_t {
private:
    Spi_t ISpi;
    void CskHi() { PinSet(ADC_GPIO, ADC_CSK); }
    void CskLo() { PinClear(ADC_GPIO, ADC_CSK); }
public:
    void Init();
    uint16_t Measure();
};

extern Adc_t Adc;

#endif /* ADC_H_ */
