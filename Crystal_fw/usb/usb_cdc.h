/*
 * usb_cdc.h
 *
 *  Created on: 03 сент. 2015 г.
 *      Author: Kreyl
 */

#ifndef USB_USB_CDC_H_
#define USB_USB_CDC_H_

#include "serial_usb.h"
#include "stdarg.h"

class UsbCDC_t {
private:
    void IPrintf(const char *format, va_list args);
public:
    void Init();
    void Connect();
    void Disconnect();
    bool IsActive() { return (SDU2.config->usbp->state == USB_ACTIVE); }
    void Printf(const char *S, ...);
    void PrintfI(const char *S, ...);
    // Inner use
    SerialUSBDriver SDU2;
};

extern UsbCDC_t UsbCDC;

#endif /* USB_USB_CDC_H_ */
