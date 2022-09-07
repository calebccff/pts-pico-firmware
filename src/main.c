#include <pico/printf.h>
#include <hardware/uart.h>
#include <pico/stdlib.h>
#include <hardware/structs/sio.h>
#include <tusb.h>

#define LED_PIN PICO_DEFAULT_LED_PIN
#define POWER_PIN 6
#define POWER_KEY_PIN 2
#define BOOTLOADER_KEY_PIN 3

#define UART_TX_PIN 4
#define UART_RX_PIN 5

#define USB_IF_CONTROL 0
#define USB_IF_UART 1

/**
 * Read a single byte from the control ttyACM device and toggle the requested
 * GPIO pins
 */
void
handle_control_rx()
{
	uint8_t buf[64];
	int readlen = tud_cdc_n_read(USB_IF_CONTROL, buf, sizeof(buf));
	if (readlen < 1) {
		return;
	}
	int cmd = buf[0];
	gpio_put(LED_PIN, 1);
	sleep_ms(50);
	gpio_put(LED_PIN, 0);
	sleep_ms(50);

	switch (cmd) {
		case 'p':
			gpio_put(POWER_PIN, 0);
			break;
		case 'P':
			gpio_put(POWER_PIN, 1);
			break;
		case 'b':
			// Float the pin
			gpio_set_dir(POWER_KEY_PIN, GPIO_IN);
			break;
		case 'B':
			// Ground the pin
			gpio_set_dir(POWER_KEY_PIN, GPIO_OUT);
			gpio_put(POWER_KEY_PIN, 0);
			break;
		case 'r':
			// Float the pin
			gpio_set_dir(BOOTLOADER_KEY_PIN, GPIO_IN);
			break;
		case 'R':
			// Ground the pin
			gpio_set_dir(BOOTLOADER_KEY_PIN, GPIO_OUT);
			gpio_put(BOOTLOADER_KEY_PIN, 0);
			break;
		default:
			tud_cdc_n_write_str(USB_IF_CONTROL, "Unknown command\n");
			tud_cdc_n_write_flush(USB_IF_CONTROL);
			break;
	}
}

/**
 * Read bytes from the second ttyACM device used for passthrough and send it
 * out through the hardware uart to the phone
 */
void
handle_passthrough_usb_rx()
{
	uint8_t buf[64];
	int readlen = tud_cdc_n_read(USB_IF_UART, buf, sizeof(buf));
	uart_write_blocking(uart1, buf, readlen);
}

/**
 * Read bytes from the hardware UART connected to the phone and send it
 * to the second ttyACM device for the passthrough
 */
void
handle_uart_rx()
{
	uint8_t buf[64];
	int uart_pos = 0;
	while (uart_is_readable(uart1) && uart_pos < 64) {
		buf[uart_pos] = uart_getc(uart1);
		uart_pos++;
	}
	tud_cdc_n_write(USB_IF_UART, buf, uart_pos);
	tud_cdc_n_write_flush(USB_IF_UART);
}

int
main()
{
	// Configure the onboard led
	gpio_init(LED_PIN);
	gpio_set_dir(LED_PIN, GPIO_OUT);

	// Configure the GPIO for hardware control
	gpio_init(POWER_PIN);
	gpio_init(POWER_KEY_PIN);
	gpio_init(BOOTLOADER_KEY_PIN);
	gpio_set_pulls(POWER_PIN, false, false);
	gpio_set_pulls(POWER_KEY_PIN, false, false);
	gpio_set_pulls(BOOTLOADER_KEY_PIN, false, false);

	// Power control pin is pushpull
	gpio_set_dir(POWER_PIN, GPIO_OUT);
	gpio_put(POWER_PIN, 0);

	// Key control pins are open drain
	gpio_set_dir(POWER_KEY_PIN, GPIO_IN);
	gpio_set_dir(BOOTLOADER_KEY_PIN, GPIO_IN);

	// Configure the hardware serial port for the passthrough
	uart_init(uart1, 115200);
	gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);
	gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
	//uart_set_hw_flow(uart1, false, false);
	//uart_set_format(uart1, 8, 1, UART_PARITY_NONE);

	// Bring up USB
	tusb_init();

	// Execute commands received from the USB serial
	// This allows setting the state for 3 pins by sending a char
	// board power supply [p = off, P = on]
	// power key [b = off, B = on]
	// bootloader key [r = off, R = on]
	while (true) {
		tud_task(); // tinyusb device task

		// Check the control serial port
		if (tud_cdc_n_available(USB_IF_CONTROL)) {
			handle_control_rx();
		}

		// Check the passthrough serial port
		if (tud_cdc_n_available(USB_IF_UART)) {
			handle_passthrough_usb_rx();
		}

		// Check the hardware uart
		if (uart_is_readable(uart1)) {
			handle_uart_rx();
		}
	}
}
