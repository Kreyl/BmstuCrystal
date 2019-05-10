#include "main.h"
#include "MsgQ.h"
#include "math.h"
#include "filter.h"
#include "usb_cdc.h"
#include "adc_ads8320.h"
#include "dac8531.h"
#include "led.h"
#include "Sequences.h"

#if 1 // ======================== Variables and defines ========================
// Forever
EvtMsgQ_t<EvtMsg_t, MAIN_EVT_Q_LEN> EvtQMain;
static const UartParams_t CmdUartParams(115200, CMD_UART_PARAMS);
CmdUart_t Uart{&CmdUartParams};
static void ITask();
static void OnCmd(Shell_t *PShell);

LedSmooth_t LedPwr(LED1_PIN);
LedSmooth_t LedUSBAct(LED2_PIN);
LedSmooth_t LedData(LED3_PIN);

// Sampling and filtering
Timer_t SamplingTmr = {SAMPLING_TMR};
static int32_t DacOutput = 0;
uint16_t ResolutionMask = 0xFFFF;
FirFloat_t Fir;
IirFloat_t Iir;
NotchFloat_t Notch;
Filter_t *PCurrentFilter = &Fir;
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

    // Leds
    LedPwr.Init();
    LedUSBAct.Init();
    LedData.Init();
    LedPwr.SetBrightness(42); // Indicate PowerOn

    // Debug pin
    PinSetupOut(DEBUG_PIN, omPushPull);

    Adc.Init();
    Dac.Init();

    // ==== Sampling timer ====
    SamplingTmr.Init();
    SamplingTmr.SetUpdateFrequencyChangingTopValue(10000); // Start Fsmpl value
    SamplingTmr.EnableIrq(SAMPLING_TMR_IRQ, IRQ_PRIO_MEDIUM);
    SamplingTmr.EnableIrqOnUpdate();
    SamplingTmr.Enable();

    // ==== USB ====
    UsbCDC.Init();
    chThdSleepMilliseconds(540);
    UsbCDC.Connect();

    // Main cycle
    ITask();
}

__noreturn
void ITask() {
    while(true) {
        EvtMsg_t Msg = EvtQMain.Fetch(TIME_INFINITE);
        switch(Msg.ID) {
            case evtIdShellCmd:
                LedData.StartOrRestart(lsqDataExch);
                OnCmd((Shell_t*)Msg.Ptr);
                ((Shell_t*)Msg.Ptr)->SignalCmdProcessed();
                break;

            case evtIdUsbReady:
                Printf("USB ready\r");
                LedUSBAct.SetBrightness(255);
                break;

            default: Printf("Unhandled Msg %u\r", Msg.ID); break;
        } // Switch
    } // while true
}

#if 1 // ======================= Command processing ============================
void OnCmd(Shell_t *PShell) {
	Cmd_t *PCmd = &PShell->Cmd;
    // Handle command
    if(PCmd->NameIs("Ping")) PShell->Ack(retvOk);

#if 1 // ==== Common ====
    else if(PCmd->NameIs("SetSmplFreq")) {
        uint32_t NewFreq;
        if(PCmd->GetNext<uint32_t>(&NewFreq) == retvOk) {
            SamplingTmr.SetUpdateFrequencyChangingTopValue(NewFreq);
            PShell->Ack(retvOk);
        }
        else PShell->Ack(retvCmdError);
    }

    else if(PCmd->NameIs("SetResolution")) {
        uint32_t NewRes;
        if(PCmd->GetNext<uint32_t>(&NewRes) == retvOk) {
            if(NewRes >= 1 and NewRes <= 16) {
                ResolutionMask = 0xFFFF << (16 - NewRes);
                PShell->Ack(retvOk);
            }
            else PShell->Ack(retvBadValue);
        }
        else PShell->Ack(retvCmdError);
    }

    // Stat/Stop
    else if(PCmd->NameIs("Start")) { PCurrentFilter->Start(); PShell->Ack(retvOk); }
    else if(PCmd->NameIs("Stop"))  { PCurrentFilter->Stop();  PShell->Ack(retvOk); }
#endif

#if 1 // ==== Float FIR ====
    else if(PCmd->NameIs("SetupFir")) {
        PCurrentFilter->Stop();
        Fir.Reset();
        Fir.ResetCoefs();
        // ==== Coeffs ====
        while(PCmd->GetNextString() == retvOk and Fir.Sz < FIR_MAX_SZ) {
            if(PCmd->Token[0] == 'a') {
                if(Convert::TryStrToFloat(&PCmd->Token[1], &Fir.a[Fir.Sz]) == retvOk) Fir.Sz++;
                else { PShell->Ack(retvBadValue); return; }    // error converting
            }
            else { PShell->Ack(retvCmdError); return; }    // != 'a'
        }
        PShell->Ack(retvOk);
        PCurrentFilter = &Fir;
        PCurrentFilter->Start();
//        Fir.PrintState();
    }
#endif

#if 1 // ==== Float IIR ====
    else if(PCmd->NameIs("SetupIir")) {
        PCurrentFilter->Stop();
        Iir.Reset();
        Iir.ResetCoefs();
        // ==== Coeffs ==== b[0] acts as b[1]
        while(PCmd->GetNextString() == retvOk) {
            if(PCmd->Token[0] == 'a') {
                if(Iir.SzA >= IIR_MAX_SZ) { PShell->Ack(retvCmdError); return; }   // Too many coefs
                if(Convert::TryStrToFloat(&PCmd->Token[1], &Iir.a[Iir.SzA]) == retvOk) Iir.SzA++;
                else { PShell->Ack(retvCmdError); return; }    // error converting
            }
            else if(PCmd->Token[0] == 'b') {
                if(Iir.SzB >= IIR_MAX_SZ) { PShell->Ack(retvCmdError); return; }   // Too many coefs
                if(Convert::TryStrToFloat(&PCmd->Token[1], &Iir.b[Iir.SzB]) == retvOk) Iir.SzB++;
                else { PShell->Ack(retvCmdError); return; }    // error converting
            }
            else { PShell->Ack(retvCmdError); return; }    // != 'a'
        }
        PShell->Ack(retvOk);
        PCurrentFilter = &Iir;
        PCurrentFilter->Start();
//        Iir.PrintState();
    }
#endif

#if 1 // ==== Notch ====
    else if(PCmd->NameIs("SetupNotch")) {
        PCurrentFilter->Stop();
        Notch.Reset();
        // ==== Coeffs ====
        float k1, k2; // Both are mandatory
        if(PCmd->GetNextString() != retvOk) { PShell->Ack(retvCmdError); return; }
        if(Convert::TryStrToFloat(PCmd->Token, &k1) != retvOk) { PShell->Ack(retvCmdError); return; }
        if(PCmd->GetNextString() != retvOk) { PShell->Ack(retvCmdError); return; }
        if(Convert::TryStrToFloat(PCmd->Token, &k2) != retvOk) { PShell->Ack(retvCmdError); return; }
        PShell->Ack(retvOk);
        Notch.Setup(k1, k2);
        PCurrentFilter = &Notch;
        PCurrentFilter->Start();
    }
#endif

    else PShell->Ack(retvCmdUnknown);
}
#endif

#if 1 // ============================= IRQ =====================================
void OnAdcMeasurementDoneI() {
    PinSetLo(DEBUG_PIN);
#if 0 // DEBUG ByPass
    DacOutput = Adc.Rslt;
#else // ==== Remove DC from input ====
    static int32_t x1 = 0, y1;
    int32_t x0 = Adc.Rslt & ResolutionMask;
    int32_t y0 = x0 - x1 + ((9999L * y1) / 10000L);
    x1 = x0;
    y1 = y0;
    // ==== Filter ====
    y0 = PCurrentFilter->AddXAndCalculate(y0);
    // ==== Output received value ====
    DacOutput = 32768L + y0;
#endif
}

// Sampling IRQ: output y0 and start new measurement. ADC will inform app when completed.
extern "C"
void SAMPLING_TMR_IRQHandler(void) {
    CH_IRQ_PROLOGUE();
    chSysLockFromISR();
    if(SAMPLING_TMR->SR & TIM_SR_UIF) {
    	SAMPLING_TMR->SR &= ~TIM_SR_UIF;
    	PinSetHi(DEBUG_PIN);
        Adc.StartDMAMeasure();
        Dac.Set(DacOutput);
    }
    chSysUnlockFromISR();
    CH_IRQ_EPILOGUE();
}
#endif
