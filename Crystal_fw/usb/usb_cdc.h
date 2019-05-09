/*
 * usb_cdc.h
 *
 *  Created on: 03 сент. 2015 г.
 *      Author: Kreyl
 */

#ifndef USB_USB_CDC_H_
#define USB_USB_CDC_H_

#include "hal_serial_usb.h"
#include "stdarg.h"
#include "shell.h"

class UsbCDC_t : public Shell_t {
private:
public:
    void Init();
    void Connect();
    void Disconnect();
    bool IsActive() { return (SDU2.config->usbp->state == USB_ACTIVE); }
    void Printf(const char *S, ...);
    // Rx
    void IRxTask();
    // Inner use
    SerialUSBDriver SDU2;
};

extern UsbCDC_t UsbCDC;

#endif /* USB_USB_CDC_H_ */
