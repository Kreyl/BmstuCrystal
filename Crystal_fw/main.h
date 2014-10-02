/*
 * main.h
 *
 *  Created on: 15 сент. 2014 г.
 *      Author: g.kruglov
 */

#ifndef MAIN_H_
#define MAIN_H_

#include "ch.h"
#include "kl_lib_f40x.h"
#include "adc_ads8320.h"
#include "dac8531.h"
#include "cmd_uart.h"
#include "evt_mask.h"
#include "filter.h"

#if 1 // ==== Leds ====
#define LEDS_GPIO           GPIOC
#define LED_YELLOW_PIN      7
#define LED_GREEN_PIN       6
#define LED_RED_PIN         8

#define LED_YELLOW_ON()     PinSet  (LEDS_GPIO, LED_YELLOW_PIN)
#define LED_YELLOW_OFF()    PinClear(LEDS_GPIO, LED_YELLOW_PIN)
#define LED_GREEN_ON()      PinSet  (LEDS_GPIO, LED_GREEN_PIN)
#define LED_GREEN_OFF()     PinClear(LEDS_GPIO, LED_GREEN_PIN)
#define LED_RED_ON()        PinSet  (LEDS_GPIO, LED_RED_PIN)
#define LED_RED_OFF()       PinClear(LEDS_GPIO, LED_RED_PIN)
#endif

#define MAX_X_CNT           11
#define MAX_Y_CNT           11

// Analog switch
#define ADG_IN1_PIN         0
#define ADG_IN2_PIN         1

class App_t {
private:
    Timer_t SamplingTmr;
    uint16_t ResolutionMask = 0xFFFF;
    int32_t DacOutput;
    FirInt_t FirInt;
    IirInt_t IirInt;
    Filter_t *PCurrentFilter = &FirInt;
    // Output switch
    void OutputFilterOn()  { PinClear(GPIOC, ADG_IN1_PIN); PinClear(GPIOC, ADG_IN2_PIN); }
    void OutputFilterOff() { PinSet  (GPIOC, ADG_IN1_PIN); PinSet  (GPIOC, ADG_IN2_PIN); }
public:
    Thread *PThread;
    void Init();
    void SignalAdcRsltReady() { chEvtSignalI(PThread, EVTMSK_ADC_READY); }
    void SignalUsbDataOut()   { chEvtSignalI(PThread, EVTMSK_USB_DATA_OUT); }
    // Events
    void OnUartCmd();
    // Inner use
    void ITask();
    void IIrqHandler();
};

extern App_t App;



#endif /* MAIN_H_ */
