# pmostest pico

This is the firmware for the Raspberry Pi Pico that is providing the
interface between the control computer and the phone under test.

The task of the Pico device is to automate pressing the buttons on the device
and act like an USB to UART adapter for the serial debug port. The Pico
also has the ability to switch off the power to the phone by interrupting
the power lines in the USB connection.

![Block diagram](https://git.sr.ht/~martijnbraam/pmostest-pico/blob/master/diagram.svg)

The Pico firmware uses tinyusb to create two USB ACM devices that will show up
as two USB serial devices on the host machine. The first port is used to
send commands to the firmware to control buttons and power. The second port
is passed through to hardware UART lines hooked up to test points in the phone.

## Control protocol

The firmware has a very simple control protocol, based on the protocol that
is used by [CDBA](https://github.com/andersson/cdba). The protocol works by
sending commands as single characters. The commands are:

```
P - Enable power to the device
p - Disable power to the device
B - Press the power button
b - Release the power button
R - Hold the key to enter the bootloader
r - Release the key to enter the bootloader
```

The uppercase character is always used to enable a function and lowercase
to disable it.

The firmware will not echo the commands or respond. Only when an invalid command
is sent an error message will be returned.

## UART passthrough

The second ttyACM device created by the firmware emulates a standard USB-to-UART
converter and passes through to `uart1` of the Pico. The port defaults to
the 9600-8-N-1 mode expected by most operating systems. At the moment it's
only possible to change the baudrate, not the start and stop bits.