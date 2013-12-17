
#include <errno.h>
#include <stdbool.h>
#include <sys/fcntl.h>
#include <unistd.h>
#include <link.h>
#include <dev/fifo.h>
#include <dev/uartfifo.h>

#include <hwpl/pio.h>
#include <hwpl/core.h>
#include <hwpl/debug.h>

#ifdef __PHY_USB
#include "link_phy_usb.h"
#endif

#define LINK_REQD_CURRENT 500

void led_priv_on(void * args){
	uint32_t * pinmask = (uint32_t *)args;
	pio_attr_t attr;
	attr.mask = (*pinmask);
	attr.mode = PIO_MODE_OUTPUT;
	hwpl_pio_setattr(1, &attr);
	hwpl_pio_setmask(1, (void*)(attr.mask));
}

void led_priv_off(void * args){
	uint32_t * pinmask = (uint32_t *)args;
	pio_attr_t attr;
	attr.mask = (*pinmask);
	hwpl_pio_clrmask(1, (void*)(attr.mask));
	attr.mode = PIO_MODE_INPUT;
	hwpl_pio_setattr(1, &attr);
}

link_phy_t link_phy_open(const char * name, int baudrate){
	link_phy_t fd;
	uint32_t pinmask;

#ifdef __PHY_USB
	fd = link_phy_usb_open();
	if( fd <  0){
		return -1;
	}

#else
	uart_attr_t attr;

	fd = open("/dev/uart0", O_RDWR);
	if( fd <  0){
		return -1;
	}

	attr.baudrate = 460800;
	attr.parity = UART_PARITY_EVEN;
	attr.pin_assign = 0;
	attr.start = UART_ATTR_START_BITS_1;
	attr.stop = UART_ATTR_STOP_BITS_2;
	attr.width = 8;
	if( ioctl(fd, I_UART_SETATTR, &attr) < 0 ){
		return -1;
	}

#endif

	pinmask = (1<<18);
	hwpl_core_privcall(led_priv_on, &pinmask);
	usleep(50*1000);
	hwpl_core_privcall(led_priv_off, &pinmask);
	pinmask = (1<<20);
	hwpl_core_privcall(led_priv_on, &pinmask);
	usleep(75*1000);
	hwpl_core_privcall(led_priv_off, &pinmask);
	pinmask = (1<<21);
	hwpl_core_privcall(led_priv_on, &pinmask);
	usleep(100*1000);
	hwpl_core_privcall(led_priv_off, &pinmask);
	pinmask = (1<<23);
	hwpl_core_privcall(led_priv_on, &pinmask);
	usleep(200*1000);
	hwpl_core_privcall(led_priv_off, &pinmask);

	link_phy_flush(fd);

	return fd;
}

int link_phy_write(link_phy_t handle, const void * buf, int nbyte){
	int ret;
	ret = write(handle, buf, nbyte);
	return ret;
}

int link_phy_read(link_phy_t handle, void * buf, int nbyte){
	int ret;
	errno = 0;
	ret = read(handle, buf, nbyte);
	return ret;
}

int link_phy_close(link_phy_t handle){
	return close(handle);
}

void link_phy_wait(int msec){
	int i;
	for(i = 0; i < msec; i++){
		usleep(msec);
	}
}

void link_phy_flush(link_phy_t handle){
	ioctl(handle, I_FIFO_FLUSH);
}



