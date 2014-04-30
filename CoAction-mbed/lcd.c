/*
 * lcd.c
 *
 *  Created on: Mar 27, 2014
 *      Author: tgil
 */

#include <errno.h>
#include <dev/mlcd.h>
#include <hwpl/pio.h>
#include <hwpl/spi.h>
#include <hwpl/tmr.h>
#include <hwdl.h>
#include <caos.h>
#include "lcd.h"

static uint8_t mem[LCD_ROWS][LCD_COLS];
static uint8_t lcd_write_state HWPL_SYS_MEM;
static uint16_t lcd_page HWPL_SYS_MEM;
static uint8_t lcd_buffer[LCD_ROWS];
static device_transfer_t op;
static uint8_t lcd_hold = 0x00;

#define LCD_TOUCH() (lcd_hold |= 0x80)
#define LCD_HOLD_COUNT() (lcd_hold & ~0x80)
#define LCD_TOUCHED() (lcd_hold & 0x80)
#define LCD_REFRESHED() (lcd_hold &= ~0x80)

static int lcd_usecond_match_event(void * context, const void * data);
static int lcd_spi_complete_event(void * context, const void * data);

static void delay(void){
	//for some devices a delay may need to be added
	int i;
	for(i=0; i < 500; i++){
		asm volatile("nop");
	}
}

static void command_mode(void){
	hwpl_pio_clrmask(LCD_SPI_A0_PORT, (void*)LCD_SPI_A0_PINMASK); //enter command mode
	delay();
}

static void data_mode(void){
	hwpl_pio_setmask(LCD_SPI_A0_PORT, (void*)LCD_SPI_A0_PINMASK); //enter data mode
	delay();
}

static void assert_cs(void){
	hwpl_pio_clrmask(LCD_SPI_CS_PORT, (void*)LCD_SPI_CS_PINMASK); //assert SPI
	delay();
}

static void deassert_cs(void){
	hwpl_pio_setmask(LCD_SPI_CS_PORT, (void*)LCD_SPI_CS_PINMASK); //de-assert SPI
	delay();
}

enum {
	LCD_WRITE_PAGE,
	LCD_WRITE_DATA
};

static void update_count(void){
	tmr_reqattr_t chan_req;
	uint32_t now;
	chan_req.channel = LCD_USECOND_OC;
	hwpl_tmr_off(LCD_USECOND_TMR, 0);
	now = hwpl_tmr_get(LCD_USECOND_TMR, 0);
	chan_req.value = now + LCD_USECOND_COUNTS;
	if( chan_req.value > CAOS_USECOND_PERIOD ){
		chan_req.value -= CAOS_USECOND_PERIOD;
	}
	hwpl_tmr_setoc(LCD_USECOND_TMR, &chan_req);
	hwpl_tmr_on(LCD_USECOND_TMR, 0);
}

int lcd_usecond_match_event(void * context, const void * data){

	//If an application is holding the LCD -- don't update
	if( lcd_hold == 0x80 ){
		LCD_REFRESHED(); //clear the touched bit

		lcd_page = 0xB0;
		lcd_write_state = LCD_WRITE_PAGE;
		lcd_buffer[0] = 0x40;

		op.callback = lcd_spi_complete_event;
		op.context = context;
		op.nbyte = 1;
		op.buf = lcd_buffer;
		op.tid = 0;
		op.loc = 0;
		op.flags = 0;

		if( hwpl_spi_write(context, &op) < 0 ){
			deassert_cs();
		}

	} else {
		update_count(); //interrupt again on next timer match
	}

	return 1; //keep interrupt in place
}

int lcd_spi_complete_event(void * context, const void * data){
	int j;
	int i;

	//deassert CS each time a SPI event completes
	deassert_cs();

	i = lcd_page - 0xB0;
	if( i == LCD_COLS ){
		lcd_hold = 0x00;
		update_count(); //interrupt again on next timer match
		return 0; //all done
	}

	switch(lcd_write_state){
	case LCD_WRITE_PAGE:

		command_mode();
		lcd_buffer[0] = lcd_page;
		lcd_buffer[1] = 0x10;
		lcd_buffer[2] = 0x00;

		op.nbyte = 3;

		assert_cs();

		lcd_write_state = LCD_WRITE_DATA;
		hwpl_spi_write(context, &op);
		return 1;
	case LCD_WRITE_DATA:

		data_mode();
		assert_cs();

		for(j=0;j<LCD_ROWS;j++){			//128 rows high
			lcd_buffer[j] = mem[LCD_ROWS - j - 1][i];
		}

		op.nbyte = LCD_ROWS;

		lcd_page++;
		lcd_write_state = LCD_WRITE_PAGE;
		hwpl_spi_write(context, &op);
		break;
	}

	return 1;
}


int lcd_open(const device_cfg_t * cfg){
	if ( hwdl_check_spi_port(cfg) < 0 ){
		return -1;
	}
	return 0;
}

int lcd_ioctl(const device_cfg_t * cfg, int request, void * ctl){
	mlcd_attr_t * attr = (mlcd_attr_t*)ctl;
	tmr_action_t action;
	pio_attr_t pattr;
	device_periph_t p;

	switch(request){
	case I_MLCD_GETATTR:
		attr->freq = LCD_FREQ; //LCD updates 30 times per second
		attr->h = LCD_HEIGHT;
		attr->w = LCD_WIDTH;
		attr->size = LCD_ROWS * LCD_COLS;
		attr->mem = mem;
		attr->hold = lcd_hold;
		attr->rows = LCD_ROWS;
		attr->cols = LCD_COLS/4;
		attr->orientation = (ORIENT_BOTTOM)|(ORIENT_LEFT);
		break;

	case I_MLCD_CLEAR:
		memset(mem, 0, LCD_ROWS * LCD_COLS);
		lcd_hold = 0x80;
		break;

	case I_MLCD_HOLD:
		if( LCD_HOLD_COUNT() < 127 ){
			lcd_hold++;
		}
		break;

	case I_MLCD_RELEASE:
		if( LCD_HOLD_COUNT() > 0 ){
			lcd_hold--;
			LCD_TOUCH();
		}
		break;

	case I_MLCD_INIT:

		//initialize the IO -- chip select first
		pattr.mask = LCD_SPI_CS_PINMASK;
		pattr.mode = PIO_MODE_OUTPUT;
		p.port = LCD_SPI_CS_PORT;
		hwpl_pio_open((device_cfg_t*)&p);
		hwpl_pio_setattr(LCD_SPI_CS_PORT, &pattr);
		hwpl_pio_setmask(LCD_SPI_CS_PORT, (void*)LCD_SPI_CS_PINMASK);

		//Now A0
		pattr.mask = LCD_SPI_A0_PINMASK;
		if( p.port != LCD_SPI_A0_PORT ){
			p.port = LCD_SPI_A0_PORT;
			hwpl_pio_open((device_cfg_t*)&p);
		}
		hwpl_pio_setattr(LCD_SPI_A0_PORT, &pattr);
		hwpl_pio_setmask(LCD_SPI_A0_PORT, (void*)LCD_SPI_A0_PINMASK);

		//configure the timer to update the LCD
		action.channel = LCD_USECOND_OC;
		action.event = TMR_ACTION_EVENT_INTERRUPT;
		action.callback = lcd_usecond_match_event;
		action.context = (void*)cfg;
		hwpl_tmr_setaction(LCD_USECOND_TMR, &action);

		const char start[] = {0xA0, 0xAE, 0xC0, 0xA2, 0x2F, 0x21, 0x81, 0x2F};
		int i;
		const char * p = (const char*)start;

		command_mode();
		hwpl_pio_clrmask(LCD_SPI_A0_PORT, (void*)LCD_SPI_A0_PINMASK); //enter command mode

		for(i=0; i < 8; i++){
			assert_cs();
			hwpl_spi_swap(LCD_SPI_PORT, (void*)(uint32_t)(*p++));
			deassert_cs();
		}

		//start the timer update to refresh the screen
		update_count();
		break;

	case I_MLCD_ON:
		command_mode();
		assert_cs();
		hwpl_spi_swap(LCD_SPI_PORT, (void*)0xAF);
		deassert_cs();
		break;

	case I_MLCD_OFF:
		command_mode();
		assert_cs();
		hwpl_spi_swap(LCD_SPI_PORT, (void*)0xAE);
		deassert_cs();
		break;

	default:
		errno = EINVAL;
		return -1;
	}


	return 0;
}

int lcd_read(const device_cfg_t * cfg, device_transfer_t * rop){
	errno = ENOTSUP;
	return -1;
}

int lcd_write(const device_cfg_t * cfg, device_transfer_t * wop){
	errno = ENOTSUP;
	return -1;
}

int lcd_close(const device_cfg_t * cfg){
	return 0;
}
