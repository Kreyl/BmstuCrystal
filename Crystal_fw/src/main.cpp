/*
 * main.cpp
 *
 *  Created on: 20 февр. 2014 г.
 *      Author: g.kruglov
 */

#include "main.h"

/*
 * DMA:
 * - DMA2 Ch3 Stream0: SPI1 RX, ADC
 * - DMA2 Ch3 Stream3: SPI1 TX, DAC
 */

App_t App;

int main(void) {
    // Setup frequency
    Clk.UpdateFreqValues();
    // Init OS
    halInit();
    chSysInit();

    // ==== Init hardware ====
    Adc.Init();
    Dac.Init();
    // Leds
    PinSetupOut(LEDS_GPIO, LED_YELLOW_PIN, omPushPull);
    PinSetupOut(LEDS_GPIO, LED_GREEN_PIN, omPushPull);
    PinSetupOut(LEDS_GPIO, LED_RED_PIN, omPushPull);

    Uart.Init(115200);
    Uart.Printf("Crystal AHB=%uMHz\r", Clk.AHBFreqHz/1000000);

    App.Init();

    // DEBUG pins
//    PinSetupAlterFunc(GPIOC, 6, omPushPull, pudNone, AF2);
    while(true) App.ITask();
}


void App_t::Init() {
    PThread = chThdSelf();
    // ==== Sampling timer ====
    SamplingTmr.Init(TIM2);
    SamplingTmr.SetUpdateFrequency(1000); // Start Fsmpl value
    SamplingTmr.EnableIrq(TIM2_IRQn, IRQ_PRIO_MEDIUM);
    SamplingTmr.EnableIrqOnUpdate();
    SamplingTmr.Enable();
    // ==== Variables ====
    pyWrite = y;
    pyRead = y;
}

void App_t::ITask() {
    uint32_t EvtMsk = chEvtWaitAny(ALL_EVENTS);
    if(EvtMsk & EVTMSK_ADC_READY) {
        y[0] = Adc.Rslt;
//        AddNewX(Adc.Rslt);

//        LED_YELLOW_ON();
        //CalculateNewY();
//        DummyY = Adc.Rslt / 2;
    }
}

void App_t::AddNewX(int32_t NewX) {
    x[xIndx] = NewX;
    xIndx++;
    if(xIndx >= MAX_X_CNT) xIndx = 0;
}

#if 1 // ============================= IRQ =====================================
// Sampling IRQ: output y0 and start new measurement. ADC will inform app when completed.
void App_t::IIrqHandler() {
    Adc.StartDMAMeasure();
    Dac.Set(y[0]);
}

#if 1 // ==== Sampling Timer =====
extern "C" {
void TIM2_IRQHandler(void) {
    CH_IRQ_PROLOGUE();
    chSysLockFromIsr();
    if(TIM2->SR & TIM_SR_UIF) {
        TIM2->SR &= ~TIM_SR_UIF;
        App.IIrqHandler();
    }
    chSysUnlockFromIsr();
    CH_IRQ_EPILOGUE();
}
}
#endif

#endif
