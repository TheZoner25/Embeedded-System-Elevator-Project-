/*
 * uart_stdio.c/h - a module adapting uart device into standard input output and error streams  
 *
 *
 * Source : https://appelsiini.net/2011/simple-usart-with-avr-libc/
 *          https://www.nongnu.org/avr-libc/user-manual/group__avr__stdio.html
 *          https://ww1.microchip.com/downloads/en/devicedoc/atmel-2549-8-bit-avr-microcontroller-atmega640-1280-1281-2560-2561_datasheet.pdf 
 */ 

// NOTE: USART = Universal Synchronous Asynchronous Receiver Transmitter
//       UART  = Universal Asynchronous Receiver Transmitter

#ifndef UART_H
#define UART_H

#include <stdio.h>

#ifndef UART_BAUD_RATE
#define UART_BAUD_RATE (9600)
#endif

extern FILE uart_output;
extern FILE uart_input;

uint8_t setup_uart_io(void);

#endif
