/*
 * main.cpp
 *
 *  Created on: 20 февр. 2014 г.
 *      Author: g.kruglov
 */

#include "ch.h"
#include "kl_lib_f40x.h"
#include "adc_ads8320.h"
#include "dac8531.h"
#include "cmd_uart.h"
#include "application.h"

/*
 * DMA:
 * - DMA2 Ch3 Stream0: SPI1 RX, ADC
 * - DMA2 Ch3 Stream3: SPI1 TX, DAC
 */


void adc_dac(){
    uint16_t z = Adc.Measure();
    Dac.Set(z);
}

int main(void) {
    // Setup frequency
    Clk.UpdateFreqValues();
    // Init OS
    halInit();
    chSysInit();

    // Init hardware
    Adc.Init();
    Dac.Init();

    Uart.Init(115200);
    Uart.Printf("Crystal AHB=%u\r", Clk.AHBFreqHz);

    App.Init();

//    PinSetupAlterFunc(GPIOC, 6, omPushPull, pudNone, AF2);
    PinSetupOut(GPIOC, 7, omPushPull, pudNone);
    PinSetupOut(GPIOC, 8, omPushPull, pudNone);
//    PinSet(GPIOC, 8);
    while(true) App.ITask();
}


