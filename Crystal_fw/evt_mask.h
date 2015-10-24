/*
 * evt_mask.h
 *
 *  Created on: Apr 12, 2013
 *      Author: g.kruglov
 */

#ifndef EVT_MASK_H_
#define EVT_MASK_H_

// Event masks
#define EVTMSK_UART_NEW_CMD     EVENT_MASK(1)

#define EVTMSK_USB_DATA_OUT     EVENT_MASK(2)
#define EVTMSK_USB_READY        EVENT_MASK(10)
#define EVTMSK_USB_SUSPEND      EVENT_MASK(11)

#endif /* EVT_MASK_H_ */
