/*
 * main.cpp
 *
 *  Created on: 20 февр. 2014 г.
 *      Author: g.kruglov
 */

#include "main.h"
#include "MsgQ.h"
#include "math.h"
#include "filter.h"
//#include "usb_cdc.h"
//#include "adc_ads8320.h"
//#include "dac8531.h"
#include "led.h"

#if 1 // ======================== Variables and defines ========================
// Forever
EvtMsgQ_t<EvtMsg_t, MAIN_EVT_Q_LEN> EvtQMain;
static const UartParams_t CmdUartParams(115200, CMD_UART_PARAMS);
CmdUart_t Uart{&CmdUartParams};
static void ITask();
static void OnCmd(Shell_t *PShell);

LedSmooth_t Led1(LED1_PIN);
LedSmooth_t Led2(LED2_PIN);
LedSmooth_t Led3(LED3_PIN);

static TmrKL_t TmrEverySecond {TIME_MS2I(1000), evtIdEverySecond, tktPeriodic};

Timer_t SamplingTmr = {SAMPLING_TMR};
static int32_t DacOutput;
uint16_t ResolutionMask = 0xFFFF;
FirFloat_t Fir;
IirFloat_t Iir;
NotchFloat_t Notch;
Filter_t *PCurrentFilter;
#endif

int main(void) {
    // ==== Setup clock frequency ====
    Clk.SetCoreClk(cclk64MHz);
    Clk.UpdateFreqValues();

    // === Init OS ===
    halInit();
    chSysInit();
    EvtQMain.Init();

    // ==== Init hardware ====
    Uart.Init();
    Printf("\r%S %S\r", APP_NAME, XSTRINGIFY(BUILD_TIME));
    Clk.PrintFreqs();

    Led1.Init();
    Led2.Init();
    Led3.Init();

//    Adc.Init();
//    Dac.Init();

    // ==== Sampling timer ====
//    SamplingTmr.Init();
//    SamplingTmr.SetUpdateFrequency(10000); // Start Fsmpl value
//    SamplingTmr.EnableIrq(SAMPLING_TMR_IRQ, IRQ_PRIO_MEDIUM);
//    SamplingTmr.EnableIrqOnUpdate();
//    SamplingTmr.Enable();

    // ==== USB ====
//    UsbCDC.Init();
//    chThdSleepMilliseconds(540);
//    UsbCDC.Connect();

    // Main cycle
    ITask();
}

__noreturn
void ITask() {
    while(true) {
        EvtMsg_t Msg = EvtQMain.Fetch(TIME_INFINITE);
        switch(Msg.ID) {
            case evtIdEverySecond:
                break;

            case evtIdShellCmd:
                OnCmd((Shell_t*)Msg.Ptr);
                ((Shell_t*)Msg.Ptr)->SignalCmdProcessed();
                break;

            case evtIdAdcReady:
    //                // ==== Remove DC from input ====
    //    //            DacOutput = Adc.Rslt; // DEBUG
    //                static int32_t x1 = 0, y1;
    //                int32_t x0 = Adc.Rslt & ResolutionMask;
    //                int32_t y0 = x0 - x1 + ((9999 * y1) / 10000);
    //                x1 = x0;
    //                y1 = y0;
    //                // ==== Filter ====
    //                y0 = PCurrentFilter->AddXAndCalculate(y0);
    //                // ==== Output received value ====
    //                DacOutput = 32768 + y0;
    //                Led[1].SetLo();
    //            }
                break;

#if 0 // ==== USB ====
        if(EvtMsk & EVTMSK_USB_READY) {
            Uart.Printf("\rUsbReady");
            Led[0].SetHi();
        }
        if(EvtMsk & EVTMSK_USB_SUSPEND) {
            Uart.Printf("\rUsbSuspend");
            Led[0].SetLo();
        }
#endif

            default: Printf("Unhandled Msg %u\r", Msg.ID); break;
        } // Switch
    } // while true
}

#if 1 // ======================= Command processing ============================
void OnCmd(Shell_t *PShell) {
    Led2.SetBrightness(255);
	Cmd_t *PCmd = &PShell->Cmd;
//    Uart.Printf("\r%S", PCmd->Name);
    // Handle command
    if(PCmd->NameIs("#Ping")) PShell->Ack(retvOk);

#if 0 // ==== Common ====
    else if(PCmd->NameIs("#SetSmplFreq")) {
        if(PCmd->GetNextNumber(&dw32) == OK) {
            SamplingTmr.SetUpdateFrequency(dw32);
            PShell->Ack(OK);
        }
        else PShell->Ack(CMD_ERROR);
    }

    else if(PCmd->NameIs("#SetResolution")) {
        if(PCmd->GetNextNumber(&dw32) == OK) {
            if(dw32 >= 1 and dw32 <= 16) {
                ResolutionMask = 0xFFFF << (16 - dw32);
                PShell->Ack(OK);
            }
            else PShell->Ack(CMD_ERROR);
        }
        else PShell->Ack(CMD_ERROR);
    }

    // Output analog filter
    else if(PCmd->NameIs("#OutFilterOn"))  { OutputFilterOn();  PShell->Ack(OK); }
    else if(PCmd->NameIs("#OutFilterOff")) { OutputFilterOff(); PShell->Ack(OK); }

    // Stat/Stop
    else if(PCmd->NameIs("#Start")) { PCurrentFilter->Start(); PShell->Ack(OK); }
    else if(PCmd->NameIs("#Stop"))  { PCurrentFilter->Stop();  PShell->Ack(OK); }
#endif

#if 0 // ==== Float FIR ====
    else if(PCmd->NameIs("#SetupFir")) {
        PCurrentFilter->Stop();
        Fir.Reset();
        Fir.ResetCoefs();
        // ==== Coeffs ====
        while(PCmd->GetNextTokenString() == OK and Fir.Sz < FIR_MAX_SZ) {
            if(PCmd->Token[0] == 'a') {
                if(Convert::TryStrToFloat(&PCmd->Token[1], &Fir.a[Fir.Sz]) == OK) Fir.Sz++;
                else { PShell->Ack(CMD_ERROR); goto CmdEnd; }    // error converting
            }
            else { PShell->Ack(CMD_ERROR); goto CmdEnd; }    // != 'a'
        }
        PShell->Ack(OK);
        PCurrentFilter = &Fir;
        PCurrentFilter->Start();
//        Fir.PrintState();
    }
#endif

#if 0 // ==== Float IIR ====
    else if(PCmd->NameIs("#SetupIir")) {
        PCurrentFilter->Stop();
        Iir.Reset();
        Iir.ResetCoefs();
        // ==== Coeffs ==== b[0] acts as b[1]
        while(PCmd->GetNextTokenString() == OK) {
//            Uart.Printf("\rToken: %S", PCmd->Token);
            if(PCmd->Token[0] == 'a') {
                if(Iir.SzA >= IIR_MAX_SZ) { PShell->Ack(CMD_ERROR); goto CmdEnd; }   // Too many coefs
                if(Convert::TryStrToFloat(&PCmd->Token[1], &Iir.a[Iir.SzA]) == OK) Iir.SzA++;
                else { PShell->Ack(CMD_ERROR); goto CmdEnd; }    // error converting
            }
            else if(PCmd->Token[0] == 'b') {
                if(Iir.SzB >= IIR_MAX_SZ) { PShell->Ack(CMD_ERROR); goto CmdEnd; }   // Too many coefs
                if(Convert::TryStrToFloat(&PCmd->Token[1], &Iir.b[Iir.SzB]) == OK) Iir.SzB++;
                else { PShell->Ack(CMD_ERROR); goto CmdEnd; }    // error converting
            }
            else { PShell->Ack(CMD_ERROR); goto CmdEnd; }    // != 'a'
        }
        PShell->Ack(OK);
        PCurrentFilter = &Iir;
        PCurrentFilter->Start();
//        Iir.PrintState();
    }
#endif

#if 0 // ==== Notch ====
    else if(PCmd->NameIs("#SetupNotch")) {
        PCurrentFilter->Stop();
        Notch.Reset();
        // ==== Coeffs ====
        float k1, k2; // Both are mandatory
        if(PCmd->GetNextTokenString() != OK)               { PShell->Ack(CMD_ERROR); goto CmdEnd; }
        if(Convert::TryStrToFloat(PCmd->Token, &k1) != OK) { PShell->Ack(CMD_ERROR); goto CmdEnd; }
        if(PCmd->GetNextTokenString() != OK)               { PShell->Ack(CMD_ERROR); goto CmdEnd; }
        if(Convert::TryStrToFloat(PCmd->Token, &k2) != OK) { PShell->Ack(CMD_ERROR); goto CmdEnd; }
        PShell->Ack(OK);
        Notch.Setup(k1, k2);
        PCurrentFilter = &Notch;
        PCurrentFilter->Start();
    }
#endif

    else PShell->Ack(retvCmdUnknown);
}
#endif

#if 0 // ============================= IRQ =====================================
// Sampling IRQ: output y0 and start new measurement. ADC will inform app when completed.

#if 1 // ==== Sampling Timer =====
extern "C" {
void SAMPLING_TMR_IRQHandler(void) {
    CH_IRQ_PROLOGUE();
    chSysLockFromISR();
    if(SAMPLING_TMR->SR & TIM_SR_UIF) {
    	SAMPLING_TMR->SR &= ~TIM_SR_UIF;
        Adc.StartDMAMeasure();
        Dac.Set(DacOutput);
        Led[1].SetHi();
    }
    chSysUnlockFromISR();
    CH_IRQ_EPILOGUE();
}
}
#endif

#endif
