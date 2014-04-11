/*
 * application.h
 *
 *  Created on: 11 апр. 2014 г.
 *      Author: g.kruglov
 */

#ifndef APPLICATION_H_
#define APPLICATION_H_

#include "ch.h"
#include "stm32f4xx.h"
#include "kl_lib_f40x.h"
#include "evt_mask.h"

#define MAX_X_CNT   11
#define MAX_Y_CNT   11

class App_t {
private:
    Timer_t SmplTmr;
    Thread *PThread;
    int32_t x[MAX_X_CNT], xIndx;
    int32_t y[MAX_Y_CNT], *pyWrite, *pyRead;
    void AddNewX(int32_t NewX);
    void CalculateNewY();

    int32_t DummyY;
    int32_t GetCurrentY() { return DummyY; }
public:
    void Init();
    void SignalAdcRsltReady() { chEvtSignalI(PThread, EVTMSK_ADC_READY); }
    // Inner use
    void ITask();
    void IIrqHandler();
};

extern App_t App;

#endif /* APPLICATION_H_ */
