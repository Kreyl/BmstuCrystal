/*
 * main.h
 *
 *  Created on: 15 сент. 2014 г.
 *      Author: g.kruglov
 */

#ifndef MAIN_H_
#define MAIN_H_

#include "ch.h"
#include "kl_lib.h"
#include "uart.h"
#include "evt_mask.h"
#include "filter.h"
#include "kl_buf.h"
#include "shell.h"
#include "board.h"

#define APP_NAME            "Crystal"
#define APP_VERSION         __DATE__ " " __TIME__

class App_t {
private:
    thread_t *PThread;
    uint16_t ResolutionMask = 0xFFFF;
public:
    void Init();
    // Output switch
    void OutputFilterOn()  { PinClear(ADG_GPIO, ADG_IN1_PIN); PinClear(ADG_GPIO, ADG_IN2_PIN); }
    void OutputFilterOff() { PinSet  (ADG_GPIO, ADG_IN1_PIN); PinSet  (ADG_GPIO, ADG_IN2_PIN); }

    // Eternal methods
    void InitThread() { PThread = chThdGetSelfX(); }
    void SignalEvt(eventmask_t Evt) {
        chSysLock();
        chEvtSignalI(PThread, Evt);
        chSysUnlock();
    }
    void SignalEvtI(eventmask_t Evt) { chEvtSignalI(PThread, Evt); }
    void OnCmd(Shell_t *PShell);
    // Inner use
    void ITask();
};

extern App_t App;

#endif /* MAIN_H_ */
