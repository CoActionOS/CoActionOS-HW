/*
 * lcd.h
 *
 *  Created on: Mar 27, 2014
 *      Author: tgil
 */

#ifndef LCD_H_
#define LCD_H_

#include <hwpl.h>
#include <stdint.h>
#include <dev/ioctl.h>

//#define SSP
#ifdef SSP
#define hwpl_spi_swap hwpl_ssp_swap
#define hwpl_spi_write hwpl_ssp_write
#endif

#define LCD_WIDTH 32
#define LCD_HEIGHT 128
#define LCD_FREQ 15
#define LCD_USECOND_COUNTS (1000000/LCD_FREQ)
#define LCD_ROWS 128
#define LCD_COLS (32/8)
#define LCD_SPI_PORT 1
#define LCD_SPI_PINASSIGN 0
#define LCD_SPI_FREQ 1000000
#define LCD_SPI_CS_PORT 0
#define LCD_SPI_CS_PIN 18
#define LCD_SPI_CS_PINMASK (1<<LCD_SPI_CS_PIN)
#define LCD_SPI_A0_PORT 0
#define LCD_SPI_A0_PIN 6
#define LCD_SPI_A0_PINMASK (1<<LCD_SPI_A0_PIN)

#define LCD_USECOND_TMR 3
#define LCD_USECOND_OC 2



int lcd_open(const device_cfg_t * cfg);
int lcd_ioctl(const device_cfg_t * cfg, int request, void * ctl);
int lcd_read(const device_cfg_t * cfg, device_transfer_t * rop);
int lcd_write(const device_cfg_t * cfg, device_transfer_t * wop);
int lcd_close(const device_cfg_t * cfg);

#define LCD_DEVICE(device_name, mode_value, uid_value, gid_value) { \
		.name = device_name, \
		DEVICE_MODE(mode_value, uid_value, gid_value, S_IFCHR), \
		DEVICE_DRIVER(lcd), \
		.cfg.periph.port = LCD_SPI_PORT, \
		.cfg.pin_assign = LCD_SPI_PINASSIGN, \
		.cfg.pcfg.spi.mode = SPI_ATTR_MODE0, \
		.cfg.pcfg.spi.cs.port = LCD_SPI_CS_PORT, \
		.cfg.pcfg.spi.cs.pin = LCD_SPI_CS_PIN, \
		.cfg.pcfg.spi.width = 8, \
		.cfg.pcfg.spi.format = SPI_ATTR_FORMAT_SPI, \
		.cfg.bitrate = LCD_SPI_FREQ, \
		.cfg.state = 0, \
		.cfg.dcfg = 0 \
}



#endif /* LCD_H_ */
