# Black_STM32F407ve

An STM32F407ve based board details of which can be found here https://os.mbed.com/users/hudakz/code/STM32F407VET6_Hello/shortlog/

This project has been generated initialy from STM32CubeMX in order to be as standard and reproducable as possible. It was generated
for the Kiel Version 5.29 (Free version) compiler.

The project contains the following :

1.  A USB Host CDC application configured to recieve NMEA sentences from a u-Blox 6 USB GPS with 
    minimal NMEA Parsing using code from Tilen Majerle.
2.  Standard UART with printf.
3.  GPIO outputs to the 2 onboard LEDS
4.  GPIO inputs connected to the 2 onboard switches.
5.  MicroSD tests with FatFS.
6.  RTC with Manually set date/time. Future code will sync time with the GPS.

Future additions will use the onboard NRF24L01 module and add SPI Flash tests.
