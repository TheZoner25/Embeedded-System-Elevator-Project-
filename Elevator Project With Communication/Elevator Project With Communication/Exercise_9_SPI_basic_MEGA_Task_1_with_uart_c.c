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
}

state_t going_down(int16_t destination_floor)
{
    while (CURRENT_FLOOR > destination_floor) {
        CURRENT_FLOOR--;
        lcd_display_floor(CURRENT_FLOOR);
        DELAY_ms(FLOOR_MOVING_SPEED_MS);
    }
}

int main(void)
{
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
    
    while (1)
    {
        int16_t destination = floor_choice();
        state = choose_direction(destination);
        
        char idle = "Idle";
       
        
        char sent_data = '5';
        char going_up_command[] = "U";
        char going_down_command[] = "D";
        
        
        uint8_t recieved_data[15];
        

        switch (state)
        {
            case IDLE:
            break;

            case GOINGUP:
            spi_master_send((uint8_t*)going_up_command, strlen(going_up_command));
            state = going_up(destination);
            break;
            
            case GOINGDOWN:
            spi_master_send((uint8_t*)going_down_command, strlen(going_down_command));
            state = going_down(destination);
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