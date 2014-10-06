/*
 * usb_serial.h
 *
 *  Created on: 05 но€б. 2013 г.
 *      Author: G.Kruglov
 */

#ifndef USB_SERIAL_H_
#define USB_SERIAL_H_

#include "usb_f4.h"
#include "cmd.h"

#define CDC_OUTQ_SZ     256
#define CDC_INQ_SZ      2048

// Cmd related
#define CDC_CMD_BUF_SZ  1024 // payload bytes
typedef Cmd_t<CDC_CMD_BUF_SZ> UsbCmd_t;

class UsbUart_t {
private:
    uint8_t OutQBuf[CDC_OUTQ_SZ], InQBuf[CDC_INQ_SZ];
    InputQueue UsbOutQueue; // From chibios' point of view, OUT data is input
    UsbCmd_t ICmd[2], *PCmdWrite = &ICmd[0];
public:
    OutputQueue UsbInQueue; // From chibios' point of view, IN data is output
    void Init();
    void SendBuf(uint8_t *PBuf, uint32_t Len) {
        chOQWriteTimeout(&UsbInQueue, PBuf, Len, TIME_INFINITE);
        Usb.PEpBulkIn->StartTransmitQueue(&UsbInQueue);
    }
    uint8_t GetByte(uint8_t *PByte, systime_t Timeout = TIME_INFINITE) {
        msg_t r = chIQGetTimeout(&UsbOutQueue, Timeout);
        if(r >= 0) {
            *PByte = (uint8_t)r;
            return OK;
        }
        else return FAILURE;
    }
    UsbCmd_t *PCmd = &ICmd[1];
    void Ack(int32_t Result) { Printf("#Ack %d\r\n", Result); }
    void Printf(const char *format, ...);
    ProcessDataResult_t ProcessOutData();
};

extern UsbUart_t UsbUart;

#endif /* USB_SERIAL_H_ */
