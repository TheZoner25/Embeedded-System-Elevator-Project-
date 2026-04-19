/*
 * uart_stdio.c/h - a module adapting uart device into standard input output and error streams  
 *
 *
 * Source : https://appelsiini.net/2011/simple-usart-with-avr-libc/
 *          https://www.nongnu.org/avr-libc/user-manual/group__avr__stdio.html
 *          https://ww1.microchip.com/downloads/en/devicedoc/atmel-2549-8-bit-avr-microcontroller-atmega640-1280-1281-2560-2561_datasheet.pdf 
 */ 

#include "mcu.h"
#include "uart.h"

#include <avr/io.h>
#include <stdio.h>

#define MYUBRR (F_CPU/16/UART_BAUD_RATE-1) ///< Define the UBRR value using the equation in datasheet p.203 Table 22-1

// NOTE: USART = Universal Synchronous Asynchronous Receiver Transmitter
//       UART  = Universal Asynchronous Receiver Transmitter

static void USART_init(uint16_t ubrr) //USART initiation p.206
{
    /* Set baud rate in the USART Baud Rate Registers (UBRR) *///datasheet p.222
    UBRR0H = (unsigned char) (ubrr >> 8); //datasheet p.206, ubbr value is 103
    UBRR0L = (unsigned char) ubrr; //datasheet p.206, , ubbr value is 103
    
    /* Enable receiver and transmitter on RX0 and TX0 */
    UCSR0B |= (1 << RXEN0) | (1 << TXEN0); //RX complete interrupt enable//Transmitter enable // datasheet p.206, p.220 
    
    /* Set frame format: 8 bit data, 2 stop bit */
    UCSR0C |= (1 << USBS0) | (3 << UCSZ00);//Stop bit selection at 2-bit// UCSZ bit setting at 8 bit//datasheet p.221 and p.222
    
}

static int USART_Transmit(char data, FILE *stream) //datasheet p.207
{
    /* Wait until the transmit buffer is empty*/
    while(!(UCSR0A & (1 << UDRE0))) //datasheet p.207, p. 219
    {
        ;
    }
        
    /* Puts the data into a buffer, then sends/transmits the data */
    UDR0 = data;

    return 0;
}

static int USART_Receive(FILE *stream) //datasheet p.210, 219
{
    /* Wait until the recieve buffer has some data*/
    while(!(UCSR0A & (1 << RXC0)))
    {
        ;
    }
    
    /* Get the received data from the buffer */
    char c = UDR0;

    // For cross-platform compatibility we treat all CR characters as LF characters. 
    // Note: This breaks sending binary data over UART.
    if (c == 13)
    	c = 10;
    
    return c;
}

// Setup the stream functions for UART, read  https://appelsiini.net/2011/simple-usart-with-avr-libc/
FILE uart_output = FDEV_SETUP_STREAM(USART_Transmit, NULL, _FDEV_SETUP_WRITE);
FILE uart_input = FDEV_SETUP_STREAM(NULL, USART_Receive, _FDEV_SETUP_READ);


uint8_t setup_uart_io(void)
{
    // initialize the UART with 9600 BAUD. Here we pass not the desired baud rate but an already configured scaler value.
    USART_init(MYUBRR);
    
    // redirect the stdin and stdout to UART functions,  read  https://appelsiini.net/2011/simple-usart-with-avr-libc/
    stdout = &uart_output;
    stdin = &uart_input;
    
    return 0;
}
