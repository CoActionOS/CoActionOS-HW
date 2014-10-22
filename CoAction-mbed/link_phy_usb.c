
#include <errno.h>
#include <stdbool.h>
#include <sys/fcntl.h>
#include <unistd.h>
#include <link_protocol.h>
#include <hwpl.h>
#include <dev/usb.h>
#include <dev/pio.h>
#include <dev/usbfifo.h>
#include <usb_dev.h>

#include <hwpl/core.h>
#include <hwpl/debug.h>

#include <usb_dev_typedefs.h>
#include <usb_dev_defs.h>

#include "config.h"
#include "link_phy_usb.h"

typedef struct HWPL_PACK {
	usb_dev_cdc_header_t header;
	usb_dev_cdc_acm_t acm;
	usb_dev_cdc_uniondescriptor_t union_descriptor;
	usb_dev_cdc_callmanagement_t call_management;
} link_cdc_acm_interface_t;

typedef struct HWPL_PACK {
	uint8_t bLength;
	uint8_t bDescriptorType;
	uint8_t bFirstInterface;
	uint8_t bInterfaceCount;
	uint8_t bFunctionClass;
	uint8_t bFunctionSubClass;
	uint8_t bFunctionProtocol;
	uint8_t iFunction;
} usb_dev_interface_assocation_t;


#define USB_PORT 0
#define USB_INTIN 0x81
#define USB_INTIN_ALT 0x84
#define USB_BULKIN (0x80|LINK_USBPHY_BULK_ENDPOINT)
#define USB_BULKOUT (LINK_USBPHY_BULK_ENDPOINT)
#define USB_BULKIN_ALT (0x80|LINK_USBPHY_BULK_ENDPOINT_ALT)
#define USB_BULKOUT_ALT (LINK_USBPHY_BULK_ENDPOINT_ALT)
#define ENDPOINT_SIZE LINK_USBPHY_BULK_ENDPOINT_SIZE

/* \details This structure defines the USB descriptors.  This
 * value is read over the control channel by the host to configure
 * the device.
 */
typedef struct HWPL_PACK {
	usb_cfg_desc_t cfg /* The configuration descriptor */;
	usb_dev_interface_assocation_t if_asso;
	usb_interface_desc_t ifcontrol /* The interface descriptor */;
	link_cdc_acm_interface_t acm /*! The CDC ACM Class descriptor */;
	usb_ep_desc_t control /* Endpoint:  Interrupt out for control packets */;
	usb_interface_desc_t ifdata /* The interface descriptor */;
	usb_ep_desc_t data_out /* Endpoint:  Bulk out */;
	usb_ep_desc_t data_in /* Endpoint:  Bulk in */;
#ifdef STDIO_VCP
	usb_dev_interface_assocation_t if_asso_alt;
	usb_interface_desc_t ifcontrol_alt /* The interface descriptor */;
	link_cdc_acm_interface_t acm_alt /*! The CDC ACM Class descriptor */;
	usb_ep_desc_t control_alt /* Endpoint:  Interrupt out for control packets */;
	usb_interface_desc_t ifdata_alt /* The interface descriptor */;
	usb_ep_desc_t data_out_alt /* Endpoint:  Bulk out */;
	usb_ep_desc_t data_in_alt /* Endpoint:  Bulk in */;
#endif
	uint8_t terminator  /* A null terminator used by the driver (required) */;
} link_cfg_desc_t;



extern const int hwpl_core_osc_freq;


#ifndef LINK_USB_VID
/*! \details This defines the USB vendor ID used by the Link protocol.
 *
 */
#define LINK_USB_VID 0x20A0
#endif

#ifndef LINK_USB_PID
/*! \details This define the USB product ID used by the Link protocol.
 *
 */

#ifdef STDIO_VCP
#define LINK_USB_PID 0x413b
#else
#define LINK_USB_PID 0x41d5
#endif
#endif

#ifndef LINK_USB_DEV_PORT
#define LINK_USB_DEV_PORT 0
#endif

#define LINK_USB_DESC_MANUFACTURER_SIZE 15
#define LINK_USB_DESC_PRODUCT_SIZE 10
#define LINK_USB_DESC_SERIAL_SIZE 16
#define LINK_USB_DESC_MANUFACTUER_STRING 'C','o','A','c','t','i','o','n','O','S',',',' ','I','n','c'
#define LINK_USB_DESC_PRODUCT_STRING 'C','o','A','c','t','i','o','n','O','S'
#define LINK_USB_DESC_SERIAL_STRING '0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0'

#define LINK_REQD_CURRENT 500


/*! \brief USB Link String Data
 * \details This structure defines the USB strings structure which includes
 * a string for the manufacturer, product, and serial number.
 */
struct HWPL_PACK link_usb_string_t {
	uint8_t bLength;
	uint8_t bDescriptorType;
	uint16_t wLANGID;
	usb_declare_string(LINK_USB_DESC_MANUFACTURER_SIZE) manufacturer;
	usb_declare_string(LINK_USB_DESC_PRODUCT_SIZE) product;
	usb_declare_string(LINK_USB_DESC_SERIAL_SIZE) serial;
};



/*! \details This variable stores the device descriptor.  It has a weak attribute and can be
 * overridden by using a user specific value
 * in the file \a devices.c.  This allows the user to change the USB vendor and product IDs.
 * All other values should be unchanged.
 *
 */
const usb_dev_desc_t link_dev_desc HWPL_WEAK = {
		.bLength = sizeof(usb_dev_desc_t),
		.bDescriptorType = USB_DEVICE_DESCRIPTOR_TYPE,
		.bcdUSB = 0x0200,
#ifdef STDIO_VCP
		.bDeviceClass = USB_DEVICE_CLASS_MISCELLANEOUS,
		.bDeviceSubClass = 2,
		.bDeviceProtocol = 1,
#else
		.bDeviceClass = USB_DEVICE_CLASS_COMMUNICATIONS,
		.bDeviceSubClass = 0,
		.bDeviceProtocol = 0,
#endif
		.bMaxPacketSize = USB_MAX_PACKET0,
		.idVendor = LINK_USB_VID,
		.idProduct = LINK_USB_PID,
		.bcdDevice = BCD_VERSION,
		.iManufacturer = 1,
		.iProduct = 2,
		.iSerialNumber = 3,
		.bNumConfigurations = 1
};

const link_cfg_desc_t link_cfg_desc HWPL_WEAK = {

		.cfg = {
				.bLength = sizeof(usb_cfg_desc_t),
				.bDescriptorType = USB_CONFIGURATION_DESCRIPTOR_TYPE,
				.wTotalLength = sizeof(link_cfg_desc_t)-1, //exclude the zero terminator
#ifdef STDIO_VCP
				.bNumInterfaces = 0x04,
#else
				.bNumInterfaces = 0x02,
#endif
				.bConfigurationValue = 0x01,
				.iConfiguration = 0x03,
				.bmAttributes = USB_CONFIG_BUS_POWERED,
				.bMaxPower = USB_CONFIG_POWER_MA( LINK_REQD_CURRENT )
		},

		.if_asso = {
				.bLength = sizeof(usb_dev_interface_assocation_t),
				.bDescriptorType = USB_INTERFACE_ASSOCIATION_DESCRIPTOR_TYPE,
				.bFirstInterface = 0,
				.bInterfaceCount = 2,
				.bFunctionClass = USB_INTERFACE_CLASS_COMMUNICATIONS,
				.bFunctionSubClass = USB_INTERFACE_SUBCLASS_ACM,
				.bFunctionProtocol = USB_INTERFACE_PROTOCOL_V25TER,
				.iFunction = 2,
		},

		.ifcontrol = {
				.bLength = sizeof(usb_interface_desc_t),
				.bDescriptorType = USB_INTERFACE_DESCRIPTOR_TYPE,
				.bInterfaceNumber = 0x00,
				.bAlternateSetting = 0x00,
				.bNumEndpoints = 0x01,
				.bInterfaceClass = USB_INTERFACE_CLASS_COMMUNICATIONS,
				.bInterfaceSubClass = USB_INTERFACE_SUBCLASS_ACM,
				.bInterfaceProtocol = USB_INTERFACE_PROTOCOL_V25TER,
				.iInterface = 0x00
		},

		.acm = {
				.header.bLength = sizeof(usb_dev_cdc_header_t),
				.header.bDescriptorType = 0x24,
				.header.bDescriptorSubType = 0x00,
				.header.bcdCDC = 0x0110,
				.acm.bFunctionLength = sizeof(usb_dev_cdc_acm_t),
				.acm.bDescriptorType = 0x24,
				.acm.bDescriptorSubType = 0x02,
				.acm.bmCapabilities = 0x00,
				.union_descriptor.bFunctionLength = sizeof(usb_dev_cdc_uniondescriptor_t),
				.union_descriptor.bDescriptorType = 0x24,
				.union_descriptor.bDescriptorSubType = 0x06,
				.union_descriptor.bMasterInterface = 0x00,
				.union_descriptor.bSlaveInterface = 0x01,
				.call_management.bFunctionLength = sizeof(usb_dev_cdc_callmanagement_t),
				.call_management.bDescriptorType = 0x24,
				.call_management.bDescriptorSubType = 0x01,
				.call_management.bmCapabilities = 0x00,
				.call_management.bDataInterface = 0x01
		},

		.control = {
				.bLength= sizeof(usb_ep_desc_t),
				.bDescriptorType=USB_ENDPOINT_DESCRIPTOR_TYPE,
				.bEndpointAddress=USB_INTIN,
				.bmAttributes=USB_ENDPOINT_TYPE_INTERRUPT,
				.wMaxPacketSize=LINK_INTERRUPT_ENDPOINT_SIZE,
				.bInterval=1
		},

		.ifdata = {
				.bLength = sizeof(usb_interface_desc_t),
				.bDescriptorType = USB_INTERFACE_DESCRIPTOR_TYPE,
				.bInterfaceNumber = 0x01,
				.bAlternateSetting = 0x00,
				.bNumEndpoints = 0x02,
				.bInterfaceClass = USB_INTERFACE_CLASS_COMMUNICATIONS_DATA,
				.bInterfaceSubClass = 0x00,
				.bInterfaceProtocol = 0x00,
				.iInterface = 0x00
		},

		.data_out = {
				.bLength= sizeof(usb_ep_desc_t),
				.bDescriptorType=USB_ENDPOINT_DESCRIPTOR_TYPE,
				.bEndpointAddress=USB_BULKOUT,
				.bmAttributes=USB_ENDPOINT_TYPE_BULK,
				.wMaxPacketSize=LINK_BULK_ENDPOINT_SIZE,
				.bInterval=1
		},

		.data_in = {
				.bLength= sizeof(usb_ep_desc_t),
				.bDescriptorType=USB_ENDPOINT_DESCRIPTOR_TYPE,
				.bEndpointAddress=USB_BULKIN,
				.bmAttributes=USB_ENDPOINT_TYPE_BULK,
				.wMaxPacketSize=LINK_BULK_ENDPOINT_SIZE,
				.bInterval=1
		},

#ifdef STDIO_VCP

		.if_asso_alt = {
				.bLength = sizeof(usb_dev_interface_assocation_t),
				.bDescriptorType = USB_INTERFACE_ASSOCIATION_DESCRIPTOR_TYPE,
				.bFirstInterface = 2,
				.bInterfaceCount = 2,
				.bFunctionClass = USB_INTERFACE_CLASS_COMMUNICATIONS,
				.bFunctionSubClass = USB_INTERFACE_SUBCLASS_ACM,
				.bFunctionProtocol = USB_INTERFACE_PROTOCOL_V25TER,
				.iFunction = 0x00,
		},

		.ifcontrol_alt = {
				.bLength = sizeof(usb_interface_desc_t),
				.bDescriptorType = USB_INTERFACE_DESCRIPTOR_TYPE,
				.bInterfaceNumber = 0x02,
				.bAlternateSetting = 0x00,
				.bNumEndpoints = 0x01,
				.bInterfaceClass = USB_INTERFACE_CLASS_COMMUNICATIONS,
				.bInterfaceSubClass = USB_INTERFACE_SUBCLASS_ACM,
				.bInterfaceProtocol = USB_INTERFACE_PROTOCOL_V25TER,
				.iInterface = 0x00
		},

		.acm_alt = {
				.header.bLength = sizeof(usb_dev_cdc_header_t),
				.header.bDescriptorType = 0x24,
				.header.bDescriptorSubType = 0x00,
				.header.bcdCDC = 0x0110,
				.acm.bFunctionLength = sizeof(usb_dev_cdc_acm_t),
				.acm.bDescriptorType = 0x24,
				.acm.bDescriptorSubType = 0x02,
				.acm.bmCapabilities = 0x00,
				.union_descriptor.bFunctionLength = sizeof(usb_dev_cdc_uniondescriptor_t),
				.union_descriptor.bDescriptorType = 0x24,
				.union_descriptor.bDescriptorSubType = 0x06,
				.union_descriptor.bMasterInterface = 0x02,
				.union_descriptor.bSlaveInterface = 0x03,
				.call_management.bFunctionLength = sizeof(usb_dev_cdc_callmanagement_t),
				.call_management.bDescriptorType = 0x24,
				.call_management.bDescriptorSubType = 0x01,
				.call_management.bmCapabilities = 0x00,
				.call_management.bDataInterface = 0x03
		},

		.control_alt = {
				.bLength= sizeof(usb_ep_desc_t),
				.bDescriptorType=USB_ENDPOINT_DESCRIPTOR_TYPE,
				.bEndpointAddress=USB_INTIN_ALT,
				.bmAttributes=USB_ENDPOINT_TYPE_INTERRUPT,
				.wMaxPacketSize=LINK_INTERRUPT_ENDPOINT_SIZE,
				.bInterval=1
		},

		.ifdata_alt = {
				.bLength = sizeof(usb_interface_desc_t),
				.bDescriptorType = USB_INTERFACE_DESCRIPTOR_TYPE,
				.bInterfaceNumber = 0x03,
				.bAlternateSetting = 0x00,
				.bNumEndpoints = 0x02,
				.bInterfaceClass = USB_INTERFACE_CLASS_COMMUNICATIONS_DATA,
				.bInterfaceSubClass = 0x00,
				.bInterfaceProtocol = 0x00,
				.iInterface = 0x00
		},

		.data_out_alt = {
				.bLength= sizeof(usb_ep_desc_t),
				.bDescriptorType=USB_ENDPOINT_DESCRIPTOR_TYPE,
				.bEndpointAddress=USB_BULKOUT_ALT,
				.bmAttributes=USB_ENDPOINT_TYPE_BULK,
				.wMaxPacketSize=LINK_BULK_ENDPOINT_SIZE,
				.bInterval=1
		},

		.data_in_alt = {
				.bLength= sizeof(usb_ep_desc_t),
				.bDescriptorType=USB_ENDPOINT_DESCRIPTOR_TYPE,
				.bEndpointAddress=USB_BULKIN_ALT,
				.bmAttributes=USB_ENDPOINT_TYPE_BULK,
				.wMaxPacketSize=LINK_BULK_ENDPOINT_SIZE,
				.bInterval=1
		},
#endif



		.terminator = 0
};



#define CONNECT_PORT "/dev/pio2"
#define CONNECT_PINMASK (1<<9)

/*! \details This variable stores the USB strings as the defaults listed below:
 * - Manufacturer: "CoActionOS, Inc"
 * - Product: "CoActionOS"
 * - Serial Number: "00000000"
 *
 * This variable has a weak attribute.  It can be overridden by using a user specific value
 * is the file \a devices.c.
 *
 */
const struct link_usb_string_t link_string_desc HWPL_WEAK = {
		.bLength = 4,
		.bDescriptorType = USB_STRING_DESCRIPTOR_TYPE,
		.wLANGID = 0x0409, //English
		.manufacturer = usb_assign_string(LINK_USB_DESC_MANUFACTURER_SIZE, LINK_USB_DESC_MANUFACTUER_STRING),
		.product = usb_assign_string(LINK_USB_DESC_PRODUCT_SIZE, LINK_USB_DESC_PRODUCT_STRING),
		.serial = usb_assign_string(LINK_USB_DESC_SERIAL_SIZE, 0) //dynamically load SN based on silicon
};



#ifdef STDIO_VCP
void init_stdio_vcp(){
	int fd;
	fd = open("/dev/stdio", O_RDWR);
	if( fd < 0 ){
		return;
	}

	ioctl(fd, I_FIFO_INIT);
	close(fd);
	return;
}
#endif


link_phy_t link_phy_usb_open(void){
	link_phy_t fd;
	pio_attr_t attr;
	usb_attr_t usb_attr;
	int fd_pio;

	//Deassert the Connect line and enable the output

	hwpl_debug("init pio\n");
	fd_pio = open(CONNECT_PORT, O_RDWR);
	if( fd_pio < 0 ){
		return -1;
	}
	attr.mask = (CONNECT_PINMASK);
	ioctl(fd_pio, I_PIO_SETMASK, (void*)attr.mask);
	attr.mode = PIO_MODE_OUTPUT | PIO_MODE_DIRONLY;
	ioctl(fd_pio, I_PIO_SETATTR, &attr);


	//open USB
	hwpl_debug("Open link-phy-usb\n");
	fd = open("/dev/link-phy-usb", O_RDWR);
	if( fd < 0 ){
		return LINK_PHY_ERROR;
	}

	//set USB attributes
	hwpl_debug("Set USB attr\n");

	usb_attr.pin_assign = 0;
	usb_attr.mode = USB_ATTR_MODE_DEVICE;
	usb_attr.crystal_freq = hwpl_core_osc_freq;
	if( ioctl(fd, I_USB_SETATTR, &usb_attr) < 0 ){
		return LINK_PHY_ERROR;
	}

	//initialize USB device
	usb_dev_init(fd, &link_dev_desc, &link_cfg_desc, &link_string_desc);
	ioctl(fd_pio, I_PIO_CLRMASK, (void*)attr.mask);

#ifdef STDIO_VCP
	init_stdio_vcp();
#endif

	return fd;
}
