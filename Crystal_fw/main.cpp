/*
 * main.cpp
 *
 *  Created on: 20 февр. 2014 г.
 *      Author: g.kruglov
 */

#include "main.h"
#include "usb_f4.h"
#include "usb_uart.h"
#include "math.h"

/*
 * DMA:
 * - DMA2 Ch3 Stream0: SPI1 RX, ADC
 * - DMA2 Ch3 Stream3: SPI1 TX, DAC
 */

App_t App;

int main(void) {
    // ==== Setup clock frequency ====
    uint8_t ClkResult = 1;
    Clk.SetupFlashLatency(64);  // Setup Flash Latency for clock in MHz
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
//    Adc.Init();
//    Dac.Init();

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
    PCurrentFilter = &FirFloat;
    // ==== Analog switch ====
    PinSetupOut(GPIOC, ADG_IN1_PIN, omPushPull, pudNone);
    PinSetupOut(GPIOC, ADG_IN2_PIN, omPushPull, pudNone);
    OutputFilterOff();
    // ==== Sampling timer ====
    SamplingTmr.Init(TIM2);
    SamplingTmr.SetUpdateFrequency(10000); // Start Fsmpl value
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
        LED_YELLOW_ON();
        // ==== Remove DC from input ====
//        DacOutput = Adc.Rslt; // DEBUG
        static int32_t x1 = 0, y1=0;
        int32_t x0 = Adc.Rslt & ResolutionMask;
        int32_t y0 = x0 - 32768;
        y0 = x0 - x1 + ((9999 * y1) / 10000);
        x1 = x0;
        y1 = y0;
        y0 = -y0;
        // ==== Filter ====
        y0 = PCurrentFilter->AddXAndCalculate(y0);
        // ==== Output received value ====
        DacOutput = 32768 + y0;
        LED_YELLOW_OFF();
        LED_RED_OFF();
    }

    if(EvtMsk & EVTMSK_USB_READY) {
        Uart.Printf("\rUsbReady");
        LED_GREEN_ON();
    }

    if(EvtMsk & EVTMSK_USB_DATA_OUT) {
        while(UsbUart.ProcessOutData() == pdrNewCmd) OnUartCmd();
    }
}

#if 1 // ======================= Command processing ============================
void App_t::OnUartCmd() {
    UsbCmd_t *PCmd = UsbUart.PCmd;
    __attribute__((unused)) int32_t dw32 = 0;  // May be unused in some configurations
    Uart.Printf("\r%S", PCmd->Name);
    // Handle command
    if(PCmd->NameIs("#Ping")) UsbUart.Ack(OK);

#if 1 // ==== Common ====
    else if(PCmd->NameIs("#SetSmplFreq")) {
        if(PCmd->TryConvertTokenToNumber(&dw32) != OK) { UsbUart.Ack(CMD_ERROR); return; }
        SamplingTmr.SetUpdateFrequency(dw32);
        UsbUart.Ack(OK);
    }

    else if(PCmd->NameIs("#SetResolution")) {
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

#if 1 // ==== Int FIR ====
    else if(PCmd->NameIs("#SetFirInt")) {
        PCurrentFilter->Stop();
        FirInt.Reset();
        FirInt.ResetCoefs();
        // Mandatory Divider
        if(PCmd->TryConvertTokenToNumber(&FirInt.Divider) != OK) { UsbUart.Ack(CMD_ERROR); return; }
        // ==== Coeffs ====
        // a[0] is mandatory
        if(PCmd->GetNextToken() != OK) { UsbUart.Ack(CMD_ERROR); return; }
        if((PCmd->Token[0] != 'a') or (Convert::TryStrToInt32(&PCmd->Token[1], &FirInt.a[0]) != OK)) { UsbUart.Ack(CMD_ERROR); return; }
        FirInt.Sz = 1;
        // Optional other coefs
        while(true) {
            if(PCmd->GetNextToken() == OK) {
                if(PCmd->Token[0] == 'a') {
                    if(Convert::TryStrToInt32(&PCmd->Token[1], &FirInt.a[FirInt.Sz]) == OK) FirInt.Sz++;
                    else { UsbUart.Ack(CMD_ERROR); return; }    // error converting
                }
                else { UsbUart.Ack(CMD_ERROR); return; }    // != 'a'
            }
            else break;   // no more coeffs
            if(FirInt.Sz >= FIR_MAX_SZ) break;
        }
        UsbUart.Ack(OK);
        PCurrentFilter = &FirInt;
        FirInt.Start();
        FirInt.PrintState();
    }
#endif

#if 1 // ==== Int IIR ====
    else if(PCmd->NameIs("#SetIirInt")) {
        PCurrentFilter->Stop();
        IirInt.Reset();
        IirInt.ResetCoefs();
        // Mandatory Divider
        if(PCmd->TryConvertTokenToNumber(&IirInt.Divider) != OK) { UsbUart.Ack(CMD_ERROR); return; }
        // ==== Coeffs ==== b[0] acts as b[1]
        // a[0] is mandatory
        if(PCmd->GetNextToken() != OK) { UsbUart.Ack(CMD_ERROR); return; }
        if((PCmd->Token[0] != 'a') or (Convert::TryStrToInt32(&PCmd->Token[1], &IirInt.a[0]) != OK)) { UsbUart.Ack(CMD_ERROR); return; }
        IirInt.SzA = 1;
        // Optional other coefs
        while(true) {
            if(PCmd->GetNextToken() == OK) {
//                Uart.Printf("\rToken: %S", PCmd->Token);
                if(PCmd->Token[0] == 'a') {
                    if(Convert::TryStrToInt32(&PCmd->Token[1], &IirInt.a[IirInt.SzA]) == OK) IirInt.SzA++;
                    else { UsbUart.Ack(CMD_ERROR); return; }    // error converting
                }
                else if(PCmd->Token[0] == 'b') {
                    if(Convert::TryStrToInt32(&PCmd->Token[1], &IirInt.b[IirInt.SzB]) == OK) IirInt.SzB++;
                    else { UsbUart.Ack(CMD_ERROR); return; }    // error converting
                }
                else { UsbUart.Ack(CMD_ERROR); return; }    // != 'a'
            }
            else break;   // no more coeffs
            if(IirInt.SzA >= IIR_MAX_SZ or IirInt.SzB >= IIR_MAX_SZ) break;
        }
        UsbUart.Ack(OK);
        PCurrentFilter = &IirInt;
        IirInt.Start();
        IirInt.PrintState();
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
    LED_RED_ON();
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
