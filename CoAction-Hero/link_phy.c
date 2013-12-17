
#include <errno.h>
#include <stdbool.h>
#include <sys/fcntl.h>
#include <unistd.h>
#include <link.h>
#include <dev/fifo.h>

#define LINK_REQD_CURRENT 500

extern link_phy_t link_phy_usb_open(const char * name, int baudrate);

link_phy_t link_phy_open(const char * name, int baudrate){
	return link_phy_usb_open(name, baudrate);
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



