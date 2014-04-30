/*
 * hio.c
 *
 *  Created on: Apr 5, 2014
 *      Author: tgil
 */


#include <errno.h>
#include <dev/hio.h>

#include <hwpl/pio.h>

#include "hio_dpad.h"

const hio_desc_t hio_dpad_desc[] = {
		{ .type = HIO_TYPE_BIT, .use = HIO_IN_DPAD, .count = 5 }, //four buttons (located in LSbs)

};

static uint8_t dpad_report(){
	uint32_t value;
	uint8_t report;

	value = hwpl_pio_get(HIO_DPAD_PORT, 0);

	report = 0;
	if( value & (1<<HIO_DPAD_PIN_UP) ){
		report |= HIO_DPAD_UP;
	}

	if( value & (1<<HIO_DPAD_PIN_DOWN) ){
		report |= HIO_DPAD_DOWN;
	}

	if( value & (1<<HIO_DPAD_PIN_LEFT) ){
		report |= HIO_DPAD_LEFT;
	}

	if( value & (1<<HIO_DPAD_PIN_RIGHT) ){
		report |= HIO_DPAD_RIGHT;
	}

	if( value & (1<<HIO_DPAD_PIN_CENTER) ){
		report |= HIO_DPAD_CENTER;
	}


	return report;
}

int hio_dpad_open(const device_cfg_t * cfg){
	return 0;
}

int hio_dpad_ioctl(const device_cfg_t * cfg, int request, void * ctl){
	pio_attr_t pattr;
	hio_attr_t * attr = (hio_attr_t *)ctl;
	uint8_t * report = (uint8_t*)ctl;

	switch(request){
	case I_HIO_INIT:
		pattr.mode = PIO_MODE_INPUT | PIO_MODE_PULLDOWN;
		pattr.mask = (1<<HIO_DPAD_PIN_UP)|
				(1<<HIO_DPAD_PIN_DOWN)|
				(1<<HIO_DPAD_PIN_LEFT)|
				(1<<HIO_DPAD_PIN_RIGHT)|
				(1<<HIO_DPAD_PIN_CENTER);

		hwpl_pio_setattr(HIO_DPAD_PORT, &pattr);


		break;
	case I_HIO_GETATTR:
		attr->report_desc = hio_dpad_desc;
		break;
	case I_HIO_REPORT:
		//copy the report to ctl

		//the report is just one byte
		*report = dpad_report();
		break;
	default:
		errno = EINVAL;
		return -1;
	}
	return 0;
}

int hio_dpad_read(const device_cfg_t * cfg, device_transfer_t * rop){
	errno = ENOTSUP;
	return -1;
}

int hio_dpad_write(const device_cfg_t * cfg, device_transfer_t * wop){
	errno = ENOTSUP;
	return -1;
}

int hio_dpad_close(const device_cfg_t * cfg){
	return 0;
}
