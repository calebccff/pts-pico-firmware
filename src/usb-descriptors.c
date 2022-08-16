#include "tusb.h"
#include <pico/unique_id.h>

#define STR_MANUFACTURER 1
#define STR_PRODUCT 2
#define STR_SERIAL 3
#define STR_CDC0 4
#define STR_CDC1 5

static char *const usb_strings[] = {
	[STR_MANUFACTURER] = "postmarketOS",
	[STR_PRODUCT] = "Test Device",
	[STR_SERIAL] = "000000000000",
	[STR_CDC0] = "Control",
	[STR_CDC1] = "Passthrough",
};

static const tusb_desc_device_t device_descriptor = {
	.bLength = sizeof(tusb_desc_device_t),
	.bDescriptorType = TUSB_DESC_DEVICE,
	.bcdUSB = 0x0200,
	.bDeviceClass = TUSB_CLASS_MISC,
	.bDeviceSubClass = MISC_SUBCLASS_COMMON,
	.bDeviceProtocol = MISC_PROTOCOL_IAD,
	.bMaxPacketSize0 = CFG_TUD_ENDPOINT0_SIZE,
	.idVendor = 0x2E8A, /* RPI foundation */
	.idProduct = 0x000A, /* PI Pico */
	.bcdDevice = 0x0100,
	.iManufacturer = STR_MANUFACTURER,
	.iProduct = STR_PRODUCT,
	.iSerialNumber = STR_SERIAL,
	.bNumConfigurations = 1,
};

#define DESCRIPTOR_LEN (TUD_CONFIG_DESC_LEN + TUD_CDC_DESC_LEN * CFG_TUD_CDC)

#define EP_OUT 0
#define EP_IN 1 << 7

#define CDC_0_EP_CMD (0x1 | EP_IN)
#define CDC_1_EP_CMD (0x4 | EP_IN)
#define CDC_0_EP_OUT (0x2 | EP_OUT)
#define CDC_1_EP_OUT (0x5 | EP_OUT)
#define CDC_0_EP_IN (0x2 | EP_IN)
#define CDC_1_EP_IN (0x5 | EP_IN)
#define CDC_BUF_CMD 8
#define CDC_BUF_SER 64


static const uint8_t config0[DESCRIPTOR_LEN] = {
	TUD_CONFIG_DESCRIPTOR(1, 4, 0, DESCRIPTOR_LEN,
		TUSB_DESC_CONFIG_ATT_REMOTE_WAKEUP, 250),

	TUD_CDC_DESCRIPTOR(0, STR_CDC0, CDC_0_EP_CMD,
		CDC_BUF_CMD, CDC_0_EP_OUT, CDC_0_EP_IN,
		CDC_BUF_SER),

	TUD_CDC_DESCRIPTOR(2, STR_CDC1, CDC_1_EP_CMD,
		CDC_BUF_CMD, CDC_1_EP_OUT, CDC_1_EP_IN,
		CDC_BUF_SER),
};


const uint8_t *
tud_descriptor_device_cb(void)
{
	return (const uint8_t *) &device_descriptor;
}

const uint8_t *
tud_descriptor_configuration_cb(uint8_t index)
{
	return config0;
}

const uint16_t *
tud_descriptor_string_cb(uint8_t index, uint16_t langid)
{
	static uint16_t buffer[20];
	uint8_t len;

	if (index == STR_SERIAL) {
		pico_get_unique_board_id_string(usb_strings[STR_SERIAL], 12);
	}

	if (index == 0) {
		buffer[1] = 0x0409; // English
		len = 1;
	} else {
		const char *str;

		if (index >= sizeof(usb_strings) / sizeof(usb_strings[0]))
			return NULL;

		str = usb_strings[index];
		for (len = 0; len < 19 && str[len]; ++len)
			buffer[1 + len] = str[len];
	}

	buffer[0] = (TUSB_DESC_STRING << 8) | (2 * len + 2);

	return buffer;
}