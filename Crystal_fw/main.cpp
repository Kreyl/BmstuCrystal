/*
 * main.cpp
 *
 *  Created on: 20 февр. 2014 г.
 *      Author: g.kruglov
 */

#include "main.h"
#include "usb_f4.h"
#include "usb_uart.h"

/*
 * DMA:
 * - DMA2 Ch3 Stream0: SPI1 RX, ADC
 * - DMA2 Ch3 Stream3: SPI1 TX, DAC
 */

App_t App;

int main(void) {
    // ==== Setup clock frequency ====
    Clk.UpdateFreqValues();
    uint8_t ClkResult = 1;
    Clk.SetupFlashLatency(48);  // Setup Flash Latency for clock in MHz
    // 8 MHz/4 = 2; 2*192 = 384; 384/8 = 48 (preAHB divider); 384/8 = 48 (USB clock)
    Clk.SetupPLLDividers(4, 192, pllSysDiv8, 8);
    // 48/1 = 48 MHz core clock. APB1 & APB2 clock derive on AHB clock; APB1max = 30MHz, APB2max = 60MHz
    // Keep APB freq at 24 MHz to left peripheral settings untouched
    Clk.SetupBusDividers(ahbDiv1, apbDiv2, apbDiv2);
    if((ClkResult = Clk.SwitchToPLL()) == 0) Clk.HSIDisable();
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
    Uart.Printf("\rCrystal AHB=%uMHz APB=%uMHz", Clk.AHBFreqHz/1000000, Clk.APB1FreqHz/1000000);

    App.Init();

    Usb.Init();
    UsbUart.Init();
    chThdSleepMilliseconds(540);
    Usb.Connect();

    // Main thread
    while(true) App.ITask();
}


void App_t::Init() {
    PThread = chThdSelf();
    // Filters init
    // ==== Sampling timer ====
    SamplingTmr.Init(TIM2);
    SamplingTmr.SetUpdateFrequency(1000); // Start Fsmpl value
    SamplingTmr.EnableIrq(TIM2_IRQn, IRQ_PRIO_MEDIUM);
    SamplingTmr.EnableIrqOnUpdate();
    SamplingTmr.Enable();
    // ==== Variables ====
//    pyWrite = y;
//    pyRead = y;
}

void App_t::ITask() {
    uint32_t EvtMsk = chEvtWaitAny(ALL_EVENTS);
    if(EvtMsk & EVTMSK_ADC_READY) {
        DacOutput = Adc.Rslt;
//        AddNewX(Adc.Rslt);

//        LED_YELLOW_ON();
        //CalculateNewY();
//        DummyY = Adc.Rslt / 2;
    }

    if(EvtMsk & EVTMSK_USB_READY) {
        Uart.Printf("\rUsbReady");
        LED_GREEN_ON();
    }

    if(EvtMsk & EVTMSK_USB_DATA_OUT) {
        while(UsbUart.ProcessOutData() == pdrNewCmd) OnUartCmd();
    }
}

void App_t::AddNewX(int32_t NewX) {
    x[xIndx] = NewX;
    xIndx++;
    if(xIndx >= MAX_X_CNT) xIndx = 0;
}

#if 1 // ======================= Command processing ============================
void App_t::OnUartCmd() {
    UsbCmd_t *PCmd = UsbUart.PCmd;
    int32_t dw32 __attribute__((unused));  // May be unused in some configurations
    Uart.Printf("\r%S", PCmd->Name);
    // Handle command
    if(PCmd->NameIs("#Ping")) UsbUart.Ack(OK);

#if 1 // ==== Common ====
    else if(PCmd->NameIs("#SetSmplFreq")) {
        if(PCmd->TryConvertTokenToNumber(&dw32) != OK) { UsbUart.Ack(CMD_ERROR); return; }
        SamplingTmr.SetUpdateFrequency(dw32);
        UsbUart.Ack(OK);
    }

    else if(PCmd->NameIs("#Start")) { PFilterCurr->Start(); UsbUart.Ack(OK); }
    else if(PCmd->NameIs("#Stop"))  { PFilterCurr->Stop();  UsbUart.Ack(OK); }
#endif

#if 1 // ==== Int FIR ====
    else if(PCmd->NameIs("#SetFirInt")) {
        Fir.Stop();
        Fir.Sz = 0;
        // Mandatory params
        if(PCmd->TryConvertTokenToNumber(&Fir.Divider) != OK) { UsbUart.Ack(CMD_ERROR); return; }
        if(PCmd->TryConvertTokenToNumber(&Fir.a[0])    != OK) { UsbUart.Ack(CMD_ERROR); return; }
        Fir.Sz = 1;
        // Optional other coefs
        for(uint32_t i=1; i<FIR_MAX_SZ; i++) {
            if(PCmd->TryConvertTokenToNumber(&Fir.a[i]) == OK) Fir.Sz++;
            else break;
        }
        UsbUart.Ack(OK);
        Fir.Start();
        Fir.PrintState();
    }

#endif
    else if(*PCmd->Name == '#') UsbUart.Ack(CMD_UNKNOWN);  // reply only #-started stuff
}
#endif

#if 1 // ============================= IRQ =====================================
// Sampling IRQ: output y0 and start new measurement. ADC will inform app when completed.
void App_t::IIrqHandler() {
    Adc.StartDMAMeasure();
    Dac.Set(DacOutput);
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
