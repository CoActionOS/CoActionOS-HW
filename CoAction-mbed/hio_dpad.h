/*
 * hio.h
 *
 *  Created on: Apr 5, 2014
 *      Author: tgil
 */

#ifndef HIO_DPAD_H_
#define HIO_DPAD_H_

#include <hwpl.h>
#include <stdint.h>
#include <dev/ioctl.h>

#define HIO_DPAD_PORT 0
#define HIO_DPAD_PIN_UP 15
#define HIO_DPAD_PIN_RIGHT 23
#define HIO_DPAD_PIN_DOWN 24
#define HIO_DPAD_PIN_LEFT 17
#define HIO_DPAD_PIN_CENTER 16

int hio_dpad_open(const device_cfg_t * cfg);
int hio_dpad_ioctl(const device_cfg_t * cfg, int request, void * ctl);
int hio_dpad_read(const device_cfg_t * cfg, device_transfer_t * rop);
int hio_dpad_write(const device_cfg_t * cfg, device_transfer_t * wop);
int hio_dpad_close(const device_cfg_t * cfg);

#define HIO_DPAD_DEVICE(device_name, mode_value, uid_value, gid_value) { \
		.name = device_name, \
		DEVICE_MODE(mode_value, uid_value, gid_value, S_IFCHR), \
		DEVICE_DRIVER(hio_dpad), \
		.cfg.periph.port = 0, \
		.cfg.pin_assign = 0, \
		.cfg.pcfg.spi.mode = 0, \
		.cfg.pcfg.spi.cs.port = 0, \
		.cfg.pcfg.spi.cs.pin = 0, \
		.cfg.pcfg.spi.width = 0, \
		.cfg.pcfg.spi.format = 0, \
		.cfg.bitrate = 0, \
		.cfg.state = 0, \
		.cfg.dcfg = 0 \
}




#endif /* HIO_DPAD_H_ */
