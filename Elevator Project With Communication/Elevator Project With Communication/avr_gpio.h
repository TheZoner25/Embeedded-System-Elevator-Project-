//
// This headeronly unit generalizes input-output operations of AVR IO to
// enable use of arrays and loops.
//
// Created by Aleksei Romanenko on 24.1.2026.
// Copyright LUT Universities, 2026
// License: MIT
//

#ifndef AVR_GPIO_H
#define AVR_GPIO_H

#include <stdio.h>
#include <avr/io.h>
#include "bit_ops.h"

typedef struct avr_gpio_tag {
    __typeof__(PORTA) *port;
    __typeof__(DDRA) *direction;
    __typeof__(PA0) pin_offset;
    __typeof__(PINA) *input;
} avr_gpio_t;

/**
 * Sets GPIO pin. Does not configure direction
 * @param gpio_pin pointer to gpio pin structure that will be set
 */
static void set_gpio(avr_gpio_t * gpio_pin) {
    SET_BIT(*(gpio_pin->port), gpio_pin->pin_offset);
}

/**
 * Clears GPIO pin. Does not configure direction
 * @param gpio_pin pointer to gpio pin structure that will be set
 */
static void clear_gpio(avr_gpio_t * gpio_pin) {
    CLEAR_BIT(*(gpio_pin->port), gpio_pin->pin_offset);
}

/**
 * Sets GPIO pin as output.
 * @param gpio_pin pointer to gpio pin structure that will be set
 */
static void set_as_output(avr_gpio_t * gpio_pin) {
    // Output
    SET_BIT(*(gpio_pin->direction), gpio_pin->pin_offset);
}

/**
 * Sets GPIO pin as input with internal pull-up disabled.
 * @param gpio_pin pointer to gpio pin structure that will be set
 */
static void set_as_input_floating(avr_gpio_t * gpio_pin) {
    // Input
    CLEAR_BIT(*(gpio_pin->direction), gpio_pin->pin_offset);
    // No pull-up
    CLEAR_BIT(*(gpio_pin->port), gpio_pin->pin_offset);
}

/**
 * Sets GPIO pin as input with internal pull-up enabled.
 * @param gpio_pin pointer to gpio pin structure that will be set
 */
static void set_as_input_pull_up(avr_gpio_t * gpio_pin) {
    // Input
    CLEAR_BIT(*(gpio_pin->direction), gpio_pin->pin_offset);
    // With pull-up
    SET_BIT(*(gpio_pin->port), gpio_pin->pin_offset);
}

/**
 * Sets GPIO pin as input with internal pull-up disabled.
 * @param gpio_pin pointer to gpio pin structure that will be set
 */
static uint8_t read_gpio(avr_gpio_t * gpio_pin) {
    uint8_t result = READ_BIT(*(gpio_pin->input), gpio_pin->pin_offset);
    printf("reading gpio pin input %d result %d \r\n", gpio_pin->pin_offset, result);
    return result;
}
#endif //GPIO_H
