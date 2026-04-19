/*
 * MEGA_last.c
 *
 * Created: 18/04/2026 03.19.25
 * Author : kadir
 */ 

/*SPI_MEGA_MASTER

 * ATmega2560 is master */

#define F_CPU 16000000UL
#define FOSC 16000000UL

#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include <string.h>
#include "uart.h"
#include "mcu.h"
#include <stdint.h>
#include "lcd.h"
#include "keypad.h"
#include "delay.h"
#include "avr_gpio.h"
#include "board_config.h"

int32_t MIN_FLOOR =  0;
int32_t MAX_FLOOR = 99;
int16_t CURRENT_FLOOR = 0;

#define DOOR_OPEN_DURATION_MS (3000)
#define DOOR_CLOSE_DURATION_MS (2000)
#define FLOOR_MOVING_SPEED_MS (3000)
#define OBSTACLE_DETECTED_DURATION_MS (3000)


// Buzzer definitions for the continuous jingle(It is connected to PE4 (PWM 2))
#define BUZZER_PIN_CON PE4
#define BUZZER_DDR_CON DDRE
#define BUZZER_PORT_CON PORTE

// Defining notes for the continuous jingle

#define NOTE_A4  100
#define NOTE_B4  45
#define NOTE_C5  250
#define NOTE_D5  450
#define NOTE_E5  790

void delay_variable_ms(uint16_t ms)
{
    while (ms--) {
        _delay_ms(1);
    }
}

void playTone(uint16_t freq, uint16_t duration)
{
    if (freq == 0) {
        delay_variable_ms(duration);
        return;
    }

    uint16_t ocr = (F_CPU / (2UL * 1024UL * freq)) - 1;

    OCR1A = ocr;

    TCCR1A = (1 << COM1A0); // toggle OC1A
    TCCR1B = (1 << WGM12) | (1 << CS12) | (1 << CS10); // CTC + prescaler 1024

    delay_variable_ms(duration);

    TCCR1B = 0;
    BUZZER_PORT_CON &= ~(1 << BUZZER_PIN_CON);
}

void play_melody(void)
{
    uint16_t melody[] = {
        NOTE_A4,NOTE_B4, NOTE_C5, NOTE_D5,
        NOTE_E5
    };

    uint16_t durations[] = {
        250,250,250,250,250,250,250,250,500
    };

    for (int i = 0; i < 5; i++) {
        playTone(melody[i], durations[i]);
        delay_variable_ms(50);
    }
}




// Button definitions for obstacle detections
#define BUTTON_PIN PB7 // Digital 13 in MEGA
#define BUTTON_DDR DDRB
#define BUTTON_PORT PORTB



typedef enum {
    IDLE = 0,
    GOINGUP = 1,
    GOINGDOWN = 2,
    DOOR_OPENING = 3,
    DOOR_CLOSING = 4,
    FAULT = 5,
} state_t;

#define NO_KEY_PRESSED  (0xFF)
#define LCD_MAX_STRING (32)

static void handle_error(uint8_t return_code)
{
    if (return_code)
    {
        while(1);
    }
}

static void write_to_lcd(const char *string) {
    uint8_t len = strnlen(string, LCD_MAX_STRING);
    if (LCD_MAX_STRING == len) {
        printf("Failed to print LCD string. Too big or lacks NULL-terminator.\r\n");
        for (uint8_t i = 0; i < len; i++)
        {
            printf("%c", string[i]);
        }
        printf("\r\n");
        handle_error(1);
    } else {
        printf("LCD output: '%s'\r\n", string);
        lcd_puts(string);
    }
}

void spi_master_send(uint8_t *data, uint8_t length)
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

void spi_master_receive(uint8_t *buffer, uint8_t length)
{
    PORTB &= ~(1 << PB0); // SS LOW

    for (uint8_t i = 0; i < length; i++)
    {
        SPDR = 0xFF; // dummy data to clock slave

        while (!(SPSR & (1 << SPIF)));

        buffer[i] = SPDR;
    }

    PORTB |= (1 << PB0); // SS HIGH

    buffer[length] = '\0'; // make it string-safe
}

static int16_t floor_choice(void)
{
    int16_t destination_floor = 0;
    
    while (1) {
        lcd_clrscr();
        write_to_lcd("Choose floor:");
        
        while (1) {
            uint8_t key = KEYPAD_GetKey();  
            if (key != NO_KEY_PRESSED)
            {
                if (key >= '0' && key <= '9') {
                    uint8_t digit = key - '0';

                    destination_floor *= 10;
                    destination_floor += digit;

                    char buffer[40];
                    snprintf(buffer, sizeof(buffer), "%d", destination_floor);

                    printf("%s", buffer);
                    lcd_gotoxy(0, 1);
                    write_to_lcd(buffer);
                }
                else if (key == '#') {
                    if (destination_floor >= MIN_FLOOR && destination_floor <= MAX_FLOOR) {
                        printf("Floor Chosen\r\n");
                        return destination_floor;
						
                    }
                }
                else if (key == '*') {
                    destination_floor = 0;
                    char buffer[3];
                    snprintf(buffer, sizeof(buffer), "%d", destination_floor);
                    printf("%s", buffer);
                    
                    lcd_clrscr();
                    lcd_gotoxy(0, 0);
                    write_to_lcd("Choose floor:");
                    lcd_gotoxy(0, 1);
                    write_to_lcd(buffer);
                }
            }
        }
    }
}



state_t choose_direction(int16_t destination_floor)
{
    if (destination_floor > CURRENT_FLOOR) {
        return GOINGUP;
    }
    else if (destination_floor < CURRENT_FLOOR) {
        return GOINGDOWN;
    }
    else if (destination_floor == CURRENT_FLOOR){
        lcd_clrscr();
        write_to_lcd("Same floor");
        DELAY_ms(2000);
        return FAULT;
    }
    return FAULT;
}

void lcd_display_floor(int16_t floor)
{
    lcd_clrscr();
    lcd_gotoxy(0, 0);
    write_to_lcd("Current floor:");
    
    lcd_gotoxy(0, 1);
    char buf[16];
    snprintf(buf, sizeof(buf), "%d", floor);
    write_to_lcd(buf);
}

state_t going_up(int16_t destination_floor)
{   
    while (CURRENT_FLOOR < destination_floor) {
        CURRENT_FLOOR++;
        lcd_display_floor(CURRENT_FLOOR);
        DELAY_ms(FLOOR_MOVING_SPEED_MS);
    }
	printf("Going up is done\r\n");
	return DOOR_OPENING;
}

state_t going_down(int16_t destination_floor)
{
    while (CURRENT_FLOOR > destination_floor) {
        CURRENT_FLOOR--;
        lcd_display_floor(CURRENT_FLOOR);
        DELAY_ms(FLOOR_MOVING_SPEED_MS);
    }
	return DOOR_OPENING;
}

int main(void)
{
    
    // Button definitions for obstacle detection

    BUTTON_DDR &= ~(1<<BUTTON_PIN); // makes the 6th pin input which is connected to button
    BUTTON_PORT |= (1<<BUTTON_PIN); // Pull-up is active: If button is not pressed HIGH(1) but if it is pressed it is LOW (0)
    
    // Button definitions for the emergency situations
    DDRH &= ~(1 << PH4); // D7 (PH4) as input
    PORTH |= (1 << PH4); // Enable pull-up

    
    
	int16_t destination; // definition of destination
    uint8_t rc = setup_uart_io();
    handle_error(rc);
    state_t state = IDLE; // initial state
	
   DDRB |= (1 << PB0) | (1 << PB1) | (1 << PB2);
   SPCR |= (1 << SPE) | (1 << MSTR) | (1 << SPR0);
    

    printf("Initializing LCD driver\r\n");
    lcd_init(LCD_DISP_ON);
    lcd_clrscr();
    write_to_lcd("Ready");
    printf("LCD Ready.\r\n");
    _delay_ms(1000);
    
    KEYPAD_Init();
	char going_up_command[] = "U";
	char going_down_command[] = "D";
	char door_opening_command[] = "O";
    char obstacle_command[] = "S";
    char door_closing_command[] = "C";
	char buffer[20];
    
    while (1)
    {
        //char idle = "Idle";
        //char sent_data = '5';        
        //uint8_t recieved_data[15];
        
		switch (state)
		{
			
			
			case IDLE:
			destination = floor_choice();
			state = choose_direction(destination);
			break;

            
			case GOINGUP:
            uint8_t key = KEYPAD_GetKey();  // get floor
			spi_master_send((uint8_t*)going_up_command, strlen(going_up_command)); // GOING UP functions
            state = going_up(destination);
            
            // Emergency situation check
            if (!(PINH & (1<<PH4)))
            {
                printf("Button for emergency is pressed.");
                // LED is still on D12 (PB6)
                DDRB |= (1 << PB6);   // set as output

                // Turn LED ON
                PORTB |= (1 << PB6);
                DELAY_ms(2000);

                // Turn LED OFF after release
                PORTB &= ~(1 << PB6);
                DELAY_ms(2000);
                
                state = FAULT;
            }
			break;
			
			case GOINGDOWN:
			spi_master_send((uint8_t*)going_down_command, strlen(going_down_command)); // GOING DOWN functions
			state = going_down(destination);
            // Emergency situation check
            if (!(PINH & (1<<PH4)))
            {
                printf("Button for emergency is pressed.");
                // LED is still on D12 (PB6)
                DDRB |= (1 << PB6);   // set as output

                // Turn LED ON
                PORTB |= (1 << PB6);
                DELAY_ms(2000);

                // Turn LED OFF after release
                PORTB &= ~(1 << PB6);
                DELAY_ms(2000);
                
                state = FAULT;
            }
			break;
			
			case DOOR_OPENING:
			spi_master_send((uint8_t*)door_opening_command, strlen(door_opening_command));
            lcd_clrscr();
            write_to_lcd("Door Opening");
            DELAY_ms(3000);
			//spi_master_receive((uint8_t*)buffer,1);
            if(!(PINB & (1<<BUTTON_PIN))){ // there was exclamation mark here
				printf("Button is pressed.");
				spi_master_send((uint8_t*)obstacle_command, strlen(obstacle_command));
				printf("Data is sent");
				
	            
	            lcd_clrscr();
	            write_to_lcd("Obstacle");
                lcd_gotoxy(0,1);
                write_to_lcd("Detected");
	            DELAY_ms(OBSTACLE_DETECTED_DURATION_MS);
				
				state = DOOR_CLOSING;
				break;
	              
            }
			
            
			else{
				state = DOOR_CLOSING;
				break;
			}
			
			
               
            case DOOR_CLOSING:
            if (!(PINH & (1<<PH4)))
            {
                printf("Button for emergency is pressed.");
                // LED is still on D12 (PB6)
                DDRB |= (1 << PB6);   // set as output

                // Turn LED ON
                PORTB |= (1 << PB6);
                DELAY_ms(2000);

                // Turn LED OFF after release
                PORTB &= ~(1 << PB6);
                
                //Writing message to LCD
                lcd_clrscr();
                write_to_lcd("Emergency!");
                DELAY_ms(2000);
                state = FAULT;
            }
            spi_master_send((uint8_t*)door_closing_command, strlen(door_closing_command)); // DOOR CLOSING functions
            lcd_clrscr();
            write_to_lcd("Door Closing");
            DELAY_ms(2000);
            state = IDLE;
            
            break;
            
                                     
                
          
			case FAULT:
			state = IDLE;
			break;

			default:
			state = IDLE;
			break;
			
		}
        
    }

    return 0; 
}