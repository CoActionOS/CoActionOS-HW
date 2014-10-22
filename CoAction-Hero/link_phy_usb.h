/*
 * link_phy_usb.h
 *
 *  Created on: Sep 28, 2013
 *      Author: tgil
 */

#ifndef LINK_PHY_USB_H_
#define LINK_PHY_USB_H_

//#define __STDIO_VCP

#define LINK_USBPHY_BULK_ENDPOINT (0x2)
#define LINK_USBPHY_BULK_ENDPOINT_ALT (0x5)
#define LINK_USBPHY_BULK_ENDPOINT_SIZE (64)


link_phy_t link_phy_usb_open(const char * name);

#endif /* LINK_PHY_USB_H_ */
