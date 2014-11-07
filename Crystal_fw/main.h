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
#define LED1_PIN            6
#define LED2_PIN            7
#define LED3_PIN            8

#define LED1_ON()           PinSet  (LEDS_GPIO, LED1_PIN)
#define LED1_OFF()          PinClear(LEDS_GPIO, LED1_PIN)
#define LED2_ON()           PinSet  (LEDS_GPIO, LED2_PIN)
#define LED2_OFF()          PinClear(LEDS_GPIO, LED2_PIN)
#define LED3_ON()           PinSet  (LEDS_GPIO, LED3_PIN)
#define LED3_OFF()          PinClear(LEDS_GPIO, LED3_PIN)
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
    FirFloat_t Fir;
    IirFloat_t Iir;
    NotchFloat_t Notch;
    Filter_t *PCurrentFilter;
    VirtualTimer Led3Tmr;
    // Output switch
    void OutputFilterOn()  { PinClear(GPIOC, ADG_IN1_PIN); PinClear(GPIOC, ADG_IN2_PIN); }
    void OutputFilterOff() { PinSet  (GPIOC, ADG_IN1_PIN); PinSet  (GPIOC, ADG_IN2_PIN); }
public:
    Thread *PThread;
    void Init();
    void SignalAdcRsltReady() { chEvtSignalI(PThread, EVTMSK_ADC_READY); }
    void SignalUsbDataOut()   { chEvtSignalI(PThread, EVTMSK_USB_DATA_OUT); }
    // Leds
    void LedBlink(uint32_t Duration_ms);
    // Events
    void OnUartCmd();
    // Inner use
    void ITask();
    void IIrqHandler();
};

extern App_t App;



#endif /* MAIN_H_ */
