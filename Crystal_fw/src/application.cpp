/*
 * application.cpp
 *
 *  Created on: 11 апр. 2014 г.
 *      Author: g.kruglov
 */

#include "application.h"
#include "adc_ads8320.h"
#include "dac8531.h"
#include "cmd_uart.h"

App_t App;

void App_t::Init() {
    // Thread
    PThread = chThdSelf();
    // Sampling timer
    SmplTmr.Init(TIM2);
    SmplTmr.SetUpdateFrequency(1000); // just in case
//    SmplTmr.SetTopValue(1000);
//    SmplTmr.SetPrescaler(199);
    SmplTmr.EnableIrq(TIM2_IRQn, IRQ_PRIO_MEDIUM);
    SmplTmr.EnableIrqOnUpdate();
    SmplTmr.Enable();
    // Variables
    pyWrite = y;
    pyRead = y;
}

void App_t::ITask() {
    uint32_t EvtMsk = chEvtWaitAny(ALL_EVENTS);
    if(EvtMsk & EVTMSK_ADC_READY) {
        AddNewX(Adc.Rslt);

        //CalculateNewY();
        DummyY = Adc.Rslt / 2;
    }
}

void App_t::AddNewX(int32_t NewX) {
    x[xIndx] = NewX;
    xIndx++;
    if(xIndx >= MAX_X_CNT) xIndx = 0;
}

// ================================= IRQ =======================================
void App_t::IIrqHandler() {
    PinToggle(GPIOC, 7);
    Dac.Set(Adc.Rslt);
    Adc.StartDMAMeasure();
//    Dac.Set(GetCurrentY());
}


extern "C" {
void TIM2_IRQHandler(void) {
    if(TIM2->SR & TIM_SR_UIF) {
        TIM2->SR &= ~TIM_SR_UIF;
        App.IIrqHandler();
    }
}
}
