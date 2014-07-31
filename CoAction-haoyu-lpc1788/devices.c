/* This is the board specific configuration file for CoActionOS.
 *
 */

#include <stdint.h>
#include <sys/lock.h>
#include <fcntl.h>
#include <errno.h>
#include <hwpl/device.h>
#include <hwdl.h>
#include <hwdl/microchip/sst25vf.h>
#include <hwdl/spansion/s29gl.h>
#include <hwdl/microchip/enc28j60.h>
#include <hwdl/sys.h>
#include <hwdl/uartfifo.h>
#include <hwdl/usbfifo.h>
#include <hwdl/fifo.h>
#include <link.h>
#include <sysfs.h>
#include <cafs_lite.h>

#include "link_phy_usb.h"


#define CAOS_SYSTEM_CLOCK 72000000
#define CAOS_SYSTEM_MEMORY_SIZE 8192


/* DO NOT MODIFY SECTION */
const int cpu_init_freq = CAOS_SYSTEM_CLOCK;
const int caoslib_system_memory_size = CAOS_SYSTEM_MEMORY_SIZE;
const uint32_t clk_nsec_div = (uint32_t)((uint64_t)1024 * 1000000000 / CAOS_SYSTEM_CLOCK);
const uint32_t clk_usec_mult = (uint32_t)(CAOS_SYSTEM_CLOCK / 1000000);
const int microcomputer_osc_freq = 12000000;
const pio_t usb_dev_connect = { .port = 0, .pin = 14 };
const pio_t gled = { .port = 2, .pin = 21 };
extern const device_t devices[];
/* END OF DO NOT MODIFY SECTION */

const uint32_t clk_usecond_tmr = 3;

const int hwpl_core_osc_freq = 12000000;
const int hwpl_core_cpu_freq = 96000000;
const int hwpl_core_periph_freq = 24000000;

const char _stdin_dev[] = "/dev/stdio-in" ;
const char _stdout_dev[] = "/dev/stdio-out";
const char _stderr_dev[] = "/dev/stdio-out";

#define USER_ROOT 0
#define GROUP_ROOT 0

/* This is the state information for the sst25vf flash IC driver.
 *
 */
sst25vf_state_t sst25vf_state HWPL_SYS_MEM;

volatile int usbdev_stat;

/* This is the configuration specific structure for the sst25vf
 * flash IC driver.
 */
const sst25vf_cfg_t sst25vf_cfg = SST25VF_DEVICE_CFG(-1, 0, -1, 0, 0, 17, 1*1024*1024);

#define UART0_DEVFIFO_BUFFER_SIZE 512
char uart0_fifo_buffer[UART0_DEVFIFO_BUFFER_SIZE];
const uartfifo_cfg_t uart0_fifo_cfg = UARTFIFO_DEVICE_CFG(0,
		uart0_fifo_buffer,
		UART0_DEVFIFO_BUFFER_SIZE);
uartfifo_state_t uart0_fifo_state HWPL_SYS_MEM;

#define UART1_DEVFIFO_BUFFER_SIZE 512
char uart1_fifo_buffer[UART0_DEVFIFO_BUFFER_SIZE];
const uartfifo_cfg_t uart1_fifo_cfg = UARTFIFO_DEVICE_CFG(1,
		uart1_fifo_buffer,
		UART1_DEVFIFO_BUFFER_SIZE);
uartfifo_state_t uart1_fifo_state HWPL_SYS_MEM;

#define USB0_DEVFIFO_BUFFER_SIZE 64
char usb0_fifo_buffer[USB0_DEVFIFO_BUFFER_SIZE];
const usbfifo_cfg_t usb0_fifo_cfg = USBFIFO_DEVICE_CFG(0,
		LINK_USBPHY_BULK_ENDPOINT,
		LINK_USBPHY_BULK_ENDPOINT_SIZE,
		usb0_fifo_buffer,
		USB0_DEVFIFO_BUFFER_SIZE);
usbfifo_state_t usb0_fifo_state HWPL_SYS_MEM;

#define STDIO_BUFFER_SIZE 128
char stdio_out_buffer[STDIO_BUFFER_SIZE];
char stdio_in_buffer[STDIO_BUFFER_SIZE];

fifo_cfg_t stdio_out_cfg = { .buffer = stdio_out_buffer, .size = STDIO_BUFFER_SIZE };
fifo_cfg_t stdio_in_cfg = { .buffer = stdio_in_buffer, .size = STDIO_BUFFER_SIZE };
fifo_state_t stdio_out_state = { .head = 0, .tail = 0, .rop = NULL, .rop_len = 0, .wop = NULL, .wop_len = 0 };
fifo_state_t stdio_in_state = {
		.head = 0, .tail = 0, .rop = NULL, .rop_len = 0, .wop = NULL, .wop_len = 0
};

enc28j60_state_t enc28j60_state;
const enc28j60_cfg_t enc28j60_cfg = { .rx_buffer_size = 1024, .tx_buffer_size = 1024 };

const s29gl_cfg_t s29gl_cfg = S29GL_DEVICE_CFG(0x80000000, 64*1024*1014, -1);

#define MEM_DEV 0

/* This is the list of devices that will show up in the /dev folder
 * automatically.  By default, the peripheral devices for the MCU are available
 * plus the devices on the microcomputer.
 */
const device_t devices[] = {
		//mcu peripherals
		DEVICE_PERIPH("mem0", hwpl_mem, 0, 0666, USER_ROOT, GROUP_ROOT, S_IFBLK),
		DEVICE_PERIPH("emc0", hwpl_emc, 0, 0666, USER_ROOT, GROUP_ROOT, S_IFBLK),
		DEVICE_PERIPH("core", hwpl_core, 0, 0666, USER_ROOT, GROUP_ROOT, S_IFCHR),
		DEVICE_PERIPH("core0", hwpl_core, 0, 0666, USER_ROOT, GROUP_ROOT, S_IFCHR),
		DEVICE_PERIPH("adc0", hwpl_adc, 0, 0666, USER_ROOT, GROUP_ROOT, S_IFCHR),
		DEVICE_PERIPH("dac0", hwpl_dac, 0, 0666, USER_ROOT, GROUP_ROOT, S_IFCHR),
		DEVICE_PERIPH("eeprom0", hwpl_eeprom, 0, 0666, USER_ROOT, GROUP_ROOT, S_IFBLK),
		DEVICE_PERIPH("eint0", hwpl_eint, 0, 0666, USER_ROOT, GROUP_ROOT, S_IFCHR),
		DEVICE_PERIPH("eint1", hwpl_eint, 1, 0666, USER_ROOT, GROUP_ROOT, S_IFCHR),
		DEVICE_PERIPH("eint2", hwpl_eint, 2, 0666, USER_ROOT, GROUP_ROOT, S_IFCHR),
		DEVICE_PERIPH("eint3", hwpl_eint, 3, 0666, USER_ROOT, GROUP_ROOT, S_IFCHR),
		DEVICE_PERIPH("pio0", hwpl_pio, 0, 0666, USER_ROOT, GROUP_ROOT, S_IFCHR),
		DEVICE_PERIPH("pio1", hwpl_pio, 1, 0666, USER_ROOT, GROUP_ROOT, S_IFCHR),
		DEVICE_PERIPH("pio2", hwpl_pio, 2, 0666, USER_ROOT, GROUP_ROOT, S_IFCHR),
		DEVICE_PERIPH("pio3", hwpl_pio, 3, 0666, USER_ROOT, GROUP_ROOT, S_IFCHR),
		DEVICE_PERIPH("pio4", hwpl_pio, 4, 0666, USER_ROOT, GROUP_ROOT, S_IFCHR),
		DEVICE_PERIPH("i2c0", hwpl_i2c, 0, 0666, USER_ROOT, GROUP_ROOT, S_IFCHR),
		DEVICE_PERIPH("i2c1", hwpl_i2c, 1, 0666, USER_ROOT, GROUP_ROOT, S_IFCHR),
		DEVICE_PERIPH("i2c2", hwpl_i2c, 2, 0666, USER_ROOT, GROUP_ROOT, S_IFCHR),
		//DEVICE_PERIPH("pwm1", hwpl_pwm, 1, 0666, USER_ROOT, GROUP_ROOT, S_IFBLK),
		//DEVICE_PERIPH("qei0", hwpl_qei, 0, 0666, USER_ROOT, GROUP_ROOT, S_IFCHR),
		DEVICE_PERIPH("rtc", hwpl_rtc, 0, 0666, USER_ROOT, GROUP_ROOT, S_IFCHR),
		DEVICE_PERIPH("spi0", hwpl_ssp, 0, 0666, USER_ROOT, GROUP_ROOT, S_IFCHR),
		DEVICE_PERIPH("spi1", hwpl_ssp, 1, 0666, USER_ROOT, GROUP_ROOT, S_IFCHR),
		DEVICE_PERIPH("spi2", hwpl_ssp, 2, 0666, USER_ROOT, GROUP_ROOT, S_IFCHR),
		DEVICE_PERIPH("tmr0", hwpl_tmr, 0, 0666, USER_ROOT, GROUP_ROOT, S_IFCHR),
		DEVICE_PERIPH("tmr1", hwpl_tmr, 1, 0666, USER_ROOT, GROUP_ROOT, S_IFCHR),
		DEVICE_PERIPH("tmr2", hwpl_tmr, 2, 0666, USER_ROOT, GROUP_ROOT, S_IFCHR),
		UARTFIFO_DEVICE("uart0", &uart0_fifo_cfg, &uart0_fifo_state, 0666, USER_ROOT, GROUP_ROOT),
		//DEVICE_PERIPH("uart0", hwpl_uart, 0, 0666, USER_ROOT, GROUP_ROOT, S_IFCHR),
		UARTFIFO_DEVICE("uart1", &uart1_fifo_cfg, &uart1_fifo_state, 0666, USER_ROOT, GROUP_ROOT),
		//DEVICE_PERIPH("uart1", hwpl_uart, 1, 0666, USER_ROOT, GROUP_ROOT, S_IFCHR),
		DEVICE_PERIPH("uart2", hwpl_uart, 2, 0666, USER_ROOT, GROUP_ROOT, S_IFCHR),
		DEVICE_PERIPH("uart3", hwpl_uart, 3, 0666, USER_ROOT, GROUP_ROOT, S_IFCHR),
		DEVICE_PERIPH("usb0", hwpl_usb, 0, 0666, USER_ROOT, GROUP_ROOT, S_IFCHR),

		//user devices
		//SST25VF_DEVICE("disk0", 2, 0, 0, 16, 80000000, &sst25vf_cfg, &sst25vf_state, 0666, USER_ROOT, GROUP_ROOT),
		//ENC28J60_DEVICE("eth0", 1, 0, 0, 6, 20000000, &enc28j60_cfg, &enc28j60_state, 0666, USER_ROOT, GROUP_ROOT),

		S29GL_DEVICE("disk0", 0, 200000000, &s29gl_cfg, 0, 0666, USER_ROOT, GROUP_ROOT),

		//FIFO buffers used for std in and std out
		FIFO_DEVICE("stdio-out", &stdio_out_cfg, &stdio_out_state, 0666, USER_ROOT, GROUP_ROOT),
		FIFO_DEVICE("stdio-in", &stdio_in_cfg, &stdio_in_state, 0666, USER_ROOT, GROUP_ROOT),

		//system devices
		USBFIFO_DEVICE("link-phy-usb", &usb0_fifo_cfg, &usb0_fifo_state, 0666, USER_ROOT, GROUP_ROOT),
		LED_DEVICE("gled", LED_ACTIVE_LOW,
				2, 21,
				LED_DEVICE_UNUSED, LED_DEVICE_UNUSED,
				LED_DEVICE_UNUSED, LED_DEVICE_UNUSED,
				LED_DEVICE_UNUSED, LED_DEVICE_UNUSED),


		SYS_DEVICE,
		DEVICE_TERMINATOR
};

extern const cafs_lite_cfg_t cafs_lite_cfg;

const sysfs_t const sysfs_list[] = {
		SYSFS_APP("/app", &(devices[MEM_DEV]), SYSFS_ALL_ACCESS), //the folder for ram/flash applications
		SYSFS_DEV("/dev", devices, SYSFS_READONLY_ACCESS), //the list of devices
		//CAFS_LITE("/home", &cafs_lite_cfg, SYSFS_ALL_ACCESS), //the list of devices
		SYSFS_ROOT("/", sysfs_list, SYSFS_READONLY_ACCESS), //the root filesystem
		SYSFS_TERMINATOR
};

open_file_t cafs_open_file; // Cannot be in HWPL_SYS_MEM because it is accessed in unpriv mode
cafs_lite_state_t cafs_lite_state;

const cafs_lite_cfg_t cafs_lite_cfg = {
		.open_file = &cafs_open_file,
		.devfs = &(sysfs_list[1]),
		.name = "disk0",
		.state = &cafs_lite_state
};








