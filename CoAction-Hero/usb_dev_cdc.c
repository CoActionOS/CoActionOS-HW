/* Copyright 2011; All Rights Reserved
 * Please see http://www.coactionos.com/license.html for
 * licensing information.
 */

#include <string.h>
#include <stdio.h>

#include <usb_dev_cdc.h>

int usb_dev_cdc_if_req(int event){
	uint32_t rate = 12000000;

	if ( (usb_setup_pkt.wIndex.u8[0] == 0) || (usb_setup_pkt.wIndex.u8[0] == 1) ||
			(usb_setup_pkt.wIndex.u8[0] == 2) || (usb_setup_pkt.wIndex.u8[0] == 3) ) { //! \todo The wIndex should equal the CDC interface number

		if ( (event == USB_SETUP_EVENT) ){
			switch(usb_setup_pkt.bRequest){
			case SET_LINE_CODING:
			case SET_COMM_FEATURE:
			case SEND_BREAK:
			case SEND_ENCAPSULATED_COMMAND:
				//need to receive information from the host
				usb_dev_std_ep0_data.dptr = usb_dev_std_ep0_buf;
				return 1;
			case SET_CONTROL_LINE_STATE:
				usb_dev_std_statusin_stage();
				return 1;
			case GET_LINE_CODING:
				usb_dev_std_ep0_data.dptr = usb_dev_std_ep0_buf;

				//copy line coding to usb_dev_std_ep0_buf
				usb_dev_std_ep0_buf[0] = (rate >>  0) & 0xFF;
				usb_dev_std_ep0_buf[1] = (rate >>  8) & 0xFF;
				usb_dev_std_ep0_buf[2] = (rate >> 16) & 0xFF;
				usb_dev_std_ep0_buf[3] = (rate >> 24) & 0xFF;  //rate
				usb_dev_std_ep0_buf[4] =  0; //stop bits 1
				usb_dev_std_ep0_buf[5] =  0; //no parity
				usb_dev_std_ep0_buf[6] =  8; //8 data bits

				usb_dev_std_statusin_stage();
				return 1;
			case CLEAR_COMM_FEATURE:
				usb_dev_std_statusin_stage();
				return 1;
			case GET_COMM_FEATURE:
				usb_dev_std_ep0_data.dptr = usb_dev_std_ep0_buf;
				//copy data to usb_dev_std_ep0_buf
				usb_dev_std_statusin_stage();
				return 1;
			case GET_ENCAPSULATED_RESPONSE:
				usb_dev_std_ep0_data.dptr = usb_dev_std_ep0_buf;
				//copy data to usb_dev_std_ep0_buf
				usb_dev_std_statusin_stage();
				return 1;
			default:
				return 0;
			}
		} else if ( event == USB_OUT_EVENT ){
			switch(usb_setup_pkt.bRequest){
			case SET_LINE_CODING:
			case SET_CONTROL_LINE_STATE:
			case SET_COMM_FEATURE:
			case SEND_ENCAPSULATED_COMMAND:
				//use data in usb_dev_std_ep0_buf to take action
				usb_dev_std_statusin_stage();
				return 1;
			default:
				return 0;
			}
		}
	}

	//The request was not handled
	return 0;
}
