#include "power_save.h"
#include <avr/io.h>
#include <avr/sleep.h>
#include <avr/interrupt.h>
#include <util/delay.h>

//CONFIG 
#define WAKEUP_DDR   DDRE
#define WAKEUP_PORT  PORTE
#define WAKEUP_PIN   PE4

// Flag to detect button activity
volatile uint8_t wake_flag = 0;//keyword, data type<stdint.h>, variable, initial

//INIT FUNCTION
void power_save_init(void)
{
    // Set PE4 as INPUT (button)
    WAKEUP_DDR &= ~(1 << WAKEUP_PIN);

    // Enable internal pull-up resistor
    // HIGH normally, LOW when button pressed
    WAKEUP_PORT |= (1 << WAKEUP_PIN);

    //External Interrupt Mask Register <avr/io.h>
    //Enable external interrupt INT4
    EIMSK |= (1 << INT4);

    //External Interrupt Control Register B <avr/io.h>
    //When the falling edge happens, the interrupt is triggered.
    EICRB |= (1 << ISC41);
    EICRB &= ~(1 << ISC40);

    // Enable global interrupts <avr/io.h> (macro)
    sei();
}


//10 SECOND WAIT FUNCTION

uint8_t wait_before_sleep(void)
{
    wake_flag = 0; // reset flag before waiting (clears old button press)

    for (uint8_t i = 0; i < 10; i++)   // loop for 10 seconds
    {
        _delay_ms(1000);               // wait 1 second

        // If button pressed during wait → cancel sleep
        if (wake_flag)
        {
            return 0;  // do NOT sleep
        }
    }

    return 1; // safe to go to sleep
}


//SLEEP FUNCTION

void enter_light_sleep(void)
{
    // Select light sleep mode (IDLE) <avr/sleep.h>
    set_sleep_mode(SLEEP_MODE_IDLE);

    sleep_enable();   // allow sleep

    sei();            // ensure interrupts are enabled, or else mcu will never wake up

    sleep_cpu();      // MCU sleeps here 

    // Code continues after wake-up, Does not sleep accidently
    sleep_disable();
}


//INTERRUPT SERVICE ROUTINE (macro) <avr/interrupt.h>
ISR(INT4_vect) //identifier, finds correct interrupt number, place function at core memory location

{
    // This runs when button is pressed
    wake_flag = 1;   // mark activity

    // This interrupt also wakes MCU automatically
}
