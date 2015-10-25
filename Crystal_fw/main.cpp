/*
 * main.cpp
 *
 *  Created on: 20 февр. 2014 г.
 *      Author: g.kruglov
 */

#include "main.h"
#include "math.h"
#include "leds.h"
#include "filter.h"
#include "usb_cdc.h"

App_t App;

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
	Cmd_t *PCmd = &PShell->Cmd;
    __attribute__((unused)) int32_t dw32 = 0;  // May be unused in some configurations
    PShell->Printf("\r%S", PCmd->Name);
    // Handle command
    if(PCmd->NameIs("Ping")) PShell->Ack(OK);

    else PShell->Ack(CMD_UNKNOWN);

    PShell->SignalCmdProcessed();
}


#if 0 // ==== Common ====
    else if(PCmd->NameIs("#SetSmplFreq")) {
        if(PCmd->GetNextToken() != OK) { UsbUart.Ack(CMD_ERROR); return; }
        if(PCmd->TryConvertTokenToNumber(&dw32) != OK) { UsbUart.Ack(CMD_ERROR); return; }
        SamplingTmr.SetUpdateFrequency(dw32);
        UsbUart.Ack(OK);
    }

    else if(PCmd->NameIs("#SetResolution")) {
        if(PCmd->GetNextToken() != OK) { UsbUart.Ack(CMD_ERROR); return; }
        if((PCmd->TryConvertTokenToNumber(&dw32) != OK) or (dw32 < 1 or dw32 > 16)) { UsbUart.Ack(CMD_ERROR); return; }
        ResolutionMask = 0xFFFF << (16 - dw32);
        UsbUart.Ack(OK);
    }

    // Output analog filter
    else if(PCmd->NameIs("#OutFilterOn"))  { OutputFilterOn();  UsbUart.Ack(OK); }
    else if(PCmd->NameIs("#OutFilterOff")) { OutputFilterOff(); UsbUart.Ack(OK); }

    // Stat/Stop
    else if(PCmd->NameIs("#Start")) { PCurrentFilter->Start(); UsbUart.Ack(OK); }
    else if(PCmd->NameIs("#Stop"))  { PCurrentFilter->Stop();  UsbUart.Ack(OK); }
#endif

#endif

#if 1 // ======================= Sample processing =============================

#endif
