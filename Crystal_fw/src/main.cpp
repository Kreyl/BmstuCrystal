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

/*
 * DMA:
 * - DMA2 Ch3 Stream0: SPI1 RX, ADC
 * - DMA2 Ch3 Stream3: SPI1 TX, DAC
 */


extern "C" {
void TIM2_IRQHandler(void) {
    if(TIM2->SR & TIM_SR_UIF) {
        TIM2->SR &= ~TIM_SR_UIF;
    }
    PinToggle(GPIOC, 7);
    PinToggle(GPIOC, 8);
}
}


void adc_dac(){
    uint16_t z = Adc.Measure();
    Dac.Set(z);
}

int main(void) {

//    RCC->CR |= RCC_CR_HSEON;
//    while(!(RCC->CR & RCC_CR_HSERDY)) {};
//    RCC->CFGR &=~ RCC_CFGR_SW;
//    RCC->CFGR |= RCC_CFGR_SW_HSE;

    Clk.UpdateFreqValues();
    // Init OS
    halInit();
    chSysInit();

    Adc.Init();
    Dac.Init();

    Uart.Init(115200);
    Uart.Printf("Crystal AHB=%u\r", Clk.AHBFreqHz);

    PinSetupAlterFunc(GPIOC, 6, omPushPull, pudNone, AF2);
    PinSetupOut(GPIOC, 7, omPushPull, pudNone);
    PinSetupOut(GPIOC, 8, omPushPull, pudNone);
    PinSet(GPIOC, 8);

    rccEnableTIM3(FALSE);
    TIM3->CR1 = TIM_CR1_CEN;
    TIM3->CCMR1 = 0b110<<4;
    TIM3->CCER = TIM_CCER_CC1E;
    TIM3->ARR = 100;
    //TIM3->CCR1 = 0;

    rccEnableTIM2(FALSE);
    TIM2->PSC =24000-1;         // ƒелитель = PSC + 1
    TIM2->ARR = 100;         // «начение сброса
    TIM2->DIER = TIM_DIER_UIE;    // ¬ключим прерывани€ от событи€ update
    TIM2->CR1 = TIM_CR1_CEN;      // ¬ключим таймер
    TIM2->CNT = 0;             // —бросим текущее значение таймера в 0

    NVIC_EnableIRQ(TIM2_IRQn);

    rccEnableDMA1(FALSE);


    while(true) {
        chThdSleepMilliseconds(450);
        Adc.StartDMAMeasure();
//        for(int i = 0; i<100; i++) {
//            TIM3->CCR1 = i;
//            chThdSleepMilliseconds(25);
//        }
//
//        for(int i = 100; i>0; i--) {
//              TIM3->CCR1 = i;
//              chThdSleepMilliseconds(25);
//        }
//        uint16_t a = Adc.Measure();
//        Uart.Printf("ADC=%u\r", a);

//        Dac.Set(i);
//        i+=1000;
//        Uart.Printf("DAC=%u\r", z);
          //adc_dac();
    }
}


