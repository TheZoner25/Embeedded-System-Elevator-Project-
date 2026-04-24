/*
 * UNO.c
 *
 * Created: 13/04/2026 18.39.15
 * Author : kadir
 */ 
/*SPI_UNO_SLAVE

 * ATmega328p is slave */

#define F_CPU 16000000UL //clock speed
#define FOSC 16000000UL // system clock frequency




// if obstacle is detected, the LEDs blinking time
#define FAULT_BLINK_PERIOD_MS (1000)
#define FAULT_BLINK_DURATION_MS (3000)

#include "uart.h" // for serial communication
#include "mcu.h"

#include <avr/io.h> // enables the pin and register
#include <util/delay.h> // delay header file
#include <stdio.h> // to print
#include <string.h> // string header file



#include "avr_gpio.h" // to enable the pins
#include "board_config.h" // includes LED pins
#include "delay.h" // delay header files

// Time details of the process
#define DOOR_OPEN_DURATION_MS (3000) 
#define DOOR_CLOSE_DURATION_MS (2000)
#define FLOOR_MOVING_SPEED_MS (3000)


// Buzzer definition (Pin 9 at B port) for obstacle detection (it operates the continuous jingle as well)
#define BUZZER_PIN PB1
#define BUZZER_DDR DDRB
#define BUZZER_PORT PORTB



// defining all the states
typedef enum { // typedef is for defining state_t (to make the code readable)
	IDLE = 0, // empty
	GOINGUP = 1,
	GOINGDOWN = 2,
	DOOR_OPENING = 3,
	DOOR_CLOSING = 4,
	FAULT = 5, // fault
} state_t; // enum includes all the possible states of elevator


// LED configurations
avr_gpio_t movement_led = { // avr_gpio_t stores all register adresses to control GPIO pin
	.port = &IO_2_PORT, // decides output value if it is 1 LED is on if it is 0 LED is off
	.direction = &IO_2_DIRECTION, // decides is pin input or output (LED is always output)
	.pin_offset = IO_2_PIN, // The correspondence pin of bit in port register (position)
	.input = &IO_2_INPUT // read
};

// Door opening
avr_gpio_t door_led = {
	.port = &IO_6_PORT,
	.direction = &IO_6_DIRECTION,
	.pin_offset = IO_6_PIN,
	.input = &IO_6_INPUT
};


// Door closing
avr_gpio_t door_close_led = {
	.port = &IO_5_PORT,
	.direction = &IO_5_DIRECTION,
	.pin_offset = IO_5_PIN,
	.input = &IO_5_INPUT
};

// obstacle detection LED
avr_gpio_t obstacle_led = { 
	.port = &IO_4_PORT,
	.direction = &IO_4_DIRECTION,
	.pin_offset = IO_4_PIN,
	.input = &IO_4_INPUT
};


// Defining notes for the song (frequency Hz)

#define NOTE_A4  440
#define NOTE_B4  494
#define NOTE_C5  523
#define NOTE_D5  587
#define NOTE_E5  659
#define NOTE_F5  698
#define NOTE_G5  784
#define NOTE_GS5 831



void spi_uno_send(uint8_t *data, uint8_t length)
{
	PORTB &= ~(1 << PB0); // SS LOW

	for (uint8_t i = 0; i < length; i++)
	{
		SPDR = data[i];

		while (!(SPSR & (1 << SPIF)));

		volatile uint8_t dummy = SPDR; // clear SPDR
	}

	PORTB |= (1 << PB0); // SS HIGH
}

void delay_variable_ms(uint16_t ms)
{
	while (ms--) {
		_delay_ms(1);
	}
}



void playTone(uint16_t freq, uint16_t duration)
{
	if (freq == 0) { // nothing is playing
		delay_variable_ms(duration);
		return;
	}

	uint16_t ocr = (F_CPU / (2UL * 1024UL * freq)) - 1;

	OCR1A = ocr;

	TCCR1A = (1 << COM1A0); // toggle OC1A
	TCCR1B = (1 << WGM12) | (1 << CS12) | (1 << CS10); // CTC + prescaler 1024

	delay_variable_ms(duration);

	TCCR1B = 0;
	BUZZER_PORT &= ~(1 << BUZZER_PIN);
}


void play_melody(void)
{
	uint16_t melody[] = {
		NOTE_E5, NOTE_D5, NOTE_C5, NOTE_B4,
		NOTE_C5, NOTE_D5, NOTE_E5, NOTE_E5, NOTE_E5
	};
   
	uint16_t durations[] = {
		250,250,250,250,250,250,250,500
	};
    printf("Buzzer Playing");
    for (int i = 0; i < 9; i++) {
        playTone(melody[i], durations[i]);
        delay_variable_ms(50);
    }      
}




// Door opening function: turns on door opening LED

static state_t door_opening(void)
{
	set_as_output(&door_led); // makes pin output
	set_gpio(&door_led); // LED ON

	
	DELAY_ms(DOOR_OPEN_DURATION_MS); // time to open the door

	clear_gpio(&door_led); // LED OFF
}


// Door closing function: turns on door closing LED
static state_t door_closing(void) 
{
	// LED on
	set_as_output(&door_close_led); // makes pin output
	set_gpio(&door_close_led); // LED ON


	DELAY_ms(DOOR_CLOSE_DURATION_MS); // time to close the door
	clear_gpio(&door_close_led); // LED OFF
}

// door closing function: turns on movement LED
static state_t movement_functions(void)
{
	set_as_output(&movement_led); // makes pin output
	set_gpio(&movement_led); // LED ON

	DELAY_ms(DOOR_CLOSE_DURATION_MS); // time to close the door
	clear_gpio(&movement_led); // LED OFF

}






int
main(void)
{
	setup_uart_io(); // initialize the serial communication at a certain baud rate
	
	
	
	DDRB  |= (1 << PB4); //  set MISO as output, pin 12 (of UNO) (PB4)*89 (PB4 is output, direction is adjusted)
	SPCR  |= (1 << SPE); // SPI is enabled
	
    // Obstacle Detection buzzer
	BUZZER_DDR |= (1 << BUZZER_PIN); // Set PB1 as output (Buzzer pin)(Direction makes output)
    
    
	
	/* Create variable data array that will be sent and received */
	char spi_send_data[20] = "Slave to master\n\r"; //to master
	char spi_receive_data[20]; //from master
	
	
	

	
	/* send message to master and receive message from master */
	while (1)
	{
        
		for(int8_t spi_data_index = 0; spi_data_index < sizeof(spi_send_data); spi_data_index++) //to read data from master
		{
			
			SPDR = spi_send_data[spi_data_index]; // Use SPI data register (SPDR) to send byte of data
			
			/* wait until the transmission is complete */

			// SPIF is flag to check the transmission continuation
			// If SPIF is 1 it means transmission is completed
			while(!(SPSR & (1 << SPIF))) // waits to complete the data transmission
			{
				; // waits
			}
			
			spi_receive_data[spi_data_index] = SPDR; // receive byte from the SPI data register
			printf("%c",spi_receive_data[spi_data_index]); // checks the received data from Putty
            
			
			// Movement conditions
			if (spi_receive_data[spi_data_index] == 'U' || spi_receive_data[spi_data_index] =='D'){
				movement_functions(); // in going up and down same LED is turned on
			}
            
            
			if(spi_receive_data[spi_data_index] == 'O' || spi_receive_data[spi_data_index] == 'S'){
			
                if(spi_receive_data[spi_data_index] == 'O'){
                    door_opening(); // turns on door opening LED
                    printf("Door is opened."); // putty	
                }

				if(spi_receive_data[spi_data_index] == 'S'){
					printf("Data Received"); // check point from putty

					set_as_output(&obstacle_led); // makes LED output

                    uint8_t count=0;// counts the blinking amount
					while (count<3){
						set_gpio(&obstacle_led); // LED ON
						DELAY_ms(FAULT_BLINK_PERIOD_MS); // blinking time

						clear_gpio(&obstacle_led); // LED OFF
                        DELAY_ms(FAULT_BLINK_PERIOD_MS); // blinking time

						count++; // increment to count the blinking
					}
                    play_melody(); // obstacle detection song
                    
                    		
					}
            }
            	
			// Door closing command
			if (spi_receive_data[spi_data_index] == 'C'){
                
				door_closing(); // Turns on door closing LED
                
			}	
            	
           
		}
	}
	
	return 0;
}