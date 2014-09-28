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

#define LED_YELLOW_ON()     PinSet(LEDS_GPIO, LED_YELLOW_PIN)
#define LED_YELLOW_OFF()    PinClear(LEDS_GPIO, LED_YELLOW_PIN)
#define LED_GREEN_ON()      PinSet(LEDS_GPIO, LED_GREEN_PIN)
#define LED_GREEN_OFF()     PinClear(LEDS_GPIO, LED_GREEN_PIN)
#define LED_RED_ON()        PinSet(LEDS_GPIO, LED_RED_PIN)
#define LED_RED_OFF()       PinClear(LEDS_GPIO, LED_RED_PIN)
#endif

#define MAX_X_CNT   11
#define MAX_Y_CNT   11

class App_t {
private:
    Timer_t SamplingTmr;
    int32_t x[MAX_X_CNT], xIndx;
    int32_t y[MAX_Y_CNT], *pyWrite, *pyRead;
    void AddNewX(int32_t NewX);
    void CalculateNewY();
    FirInt_t Fir;
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
