/*
 * main.cpp
 *
 *  Created on: 20 ����. 2014 �.
 *      Author: g.kruglov
 */

#include "main.h"
#include "math.h"
#include "leds.h"
#include "filter.h"
#include "usb_cdc.h"
#include "adc_ads8320.h"
#include "dac8531.h"

App_t App;
Timer_t SamplingTmr = {SAMPLING_TMR};

static int32_t DacOutput;

int main(void) {
    // ==== Setup clock frequency ====
    uint8_t ClkResult = 1;
    Clk.SetupFlashLatency(64);  // Setup Flash Latency for clock in MHz
    Clk.EnablePrefetch();
    // 8 MHz/4 = 2; 2*192 = 384; 384/6 = 64 (preAHB divider); 384/8 = 48 (USB clock)
    Clk.SetupPLLDividers(4, 192, pllSysDiv6, 8);
    // 64/1 = 64 MHz core clock. APB1 & APB2 clock derive on AHB clock; APB1max = 42MHz, APB2max = 84MHz
    // Keep APB freq at 32 MHz to left peripheral settings untouched
    Clk.SetupBusDividers(ahbDiv1, apbDiv2, apbDiv2);
    if((ClkResult = Clk.SwitchToPLL()) == 0) Clk.HSIDisable();
    Clk.UpdateFreqValues();

    // Init OS
    halInit();
    chSysInit();

    // ==== Init hardware ====
    Uart.Init(115200, UART_GPIO, UART_TX_PIN, UART_GPIO, UART_RX_PIN);
    Uart.Printf("\r%S %S", APP_NAME, APP_VERSION);
    Clk.PrintFreqs();
    if(ClkResult != 0) Uart.Printf("\rXTAL failure");

    App.InitThread();
    // Leds
    for(uint8_t i=0; i<LED_CNT; i++) Led[i].Init();

    Adc.Init();
    Dac.Init();

    // ==== Analog switch ====
    PinSetupOut(ADG_GPIO, ADG_IN1_PIN, omPushPull, pudNone);
    PinSetupOut(ADG_GPIO, ADG_IN2_PIN, omPushPull, pudNone);
    App.OutputFilterOff();

    // ==== Sampling timer ====
    SamplingTmr.Init();
    SamplingTmr.SetUpdateFrequency(10000); // Start Fsmpl value
    SamplingTmr.EnableIrq(SAMPLING_TMR_IRQ, IRQ_PRIO_MEDIUM);
    SamplingTmr.EnableIrqOnUpdate();
    SamplingTmr.Enable();

    // ==== USB ====
    UsbCDC.Init();
    chThdSleepMilliseconds(540);
    UsbCDC.Connect();

    // Main cycle
    App.ITask();
}

__attribute__ ((__noreturn__))
void App_t::ITask() {
    while(true) {
        uint32_t EvtMsk = chEvtWaitAny(ALL_EVENTS);
#if 1 // ==== USB ====
        if(EvtMsk & EVTMSK_USB_READY) {
            Uart.Printf("\rUsbReady");
            Led[0].SetHi();
        }
        if(EvtMsk & EVTMSK_USB_SUSPEND) {
            Uart.Printf("\rUsbSuspend");
            Led[0].SetLo();
        }
#endif

        if(EvtMsk & EVTMSK_ADC_READY) {
            // ==== Remove DC from input ====
//            DacOutput = Adc.Rslt; // DEBUG
            static int32_t x1 = 0, y1;
            int32_t x0 = Adc.Rslt & ResolutionMask;
            int32_t y0 = x0 - x1 + ((9999 * y1) / 10000);
            x1 = x0;
            y1 = y0;
            // ==== Filter ====
            y0 = PCurrentFilter->AddXAndCalculate(y0);
            // ==== Output received value ====
            DacOutput = 32768 + y0;
        	Led[1].SetLo();
        }

        if(EvtMsk & EVTMSK_UART_NEW_CMD) {
            OnCmd((Shell_t*)&Uart);
        }
        if(EvtMsk & EVTMSK_USB_NEW_CMD) {
            OnCmd((Shell_t*)&UsbCDC);
        }

    } // while true
}

#if 1 // ======================= Command processing ============================
void App_t::OnCmd(Shell_t *PShell) {
    Led[2].SetHi();
	Cmd_t *PCmd = &PShell->Cmd;
    __attribute__((unused)) int32_t dw32 = 0;  // May be unused in some configurations
//    Uart.Printf("\r%S", PCmd->Name);
    // Handle command
    if(PCmd->NameIs("#Ping")) PShell->Ack(OK);

#if 1 // ==== Common ====
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

#if 1 // ==== Float FIR ====
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

#if 1 // ==== Float IIR ====
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

#if 1 // ==== Notch ====
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

    else PShell->Ack(CMD_UNKNOWN);
CmdEnd:
    Led[2].SetLo();
    PShell->SignalCmdProcessed();
}
#endif

#if 1 // ============================= IRQ =====================================
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

#if 1 // ======================= Sample processing =============================

#endif