/*
 * main.cpp
 *
 *  Created on: 20 февр. 2014 г.
 *      Author: g.kruglov
 */

#include "ch.h"
#include "kl_lib_f40x.h"

static WORKING_AREA(waMyThread, 256);
static void MyThread(void *arg) {
    chRegSetThreadName("My");
    while(true) {
        PinSet(GPIOC, 7);
        chThdSleepMilliseconds(504);
        PinClear(GPIOC, 7);
        chThdSleepMilliseconds(504);
    }
}

int main(void) {
    Clk.UpdateFreqValues();
    // Init OS
    halInit();
    chSysInit();

    PinSetupAlterFunc(GPIOC, 6, omPushPull, pudNone, AF2);
    rccEnableTIM3(FALSE);
    TIM3->CR1 = TIM_CR1_CEN;
    TIM3->CCMR1 = 0b110<<4;
    TIM3->CCER = TIM_CCER_CC1E;
    TIM3->ARR = 100;
    TIM3->CCR1 = 4;

    PinSetupOut(GPIOC, 7, omPushPull, pudNone);

    //chThdCreateStatic(waMyThread, sizeof(waMyThread), NORMALPRIO, (tfunc_t)MyThread, NULL);

    while(true) {
        chThdSleepMilliseconds(999);
    }
}

