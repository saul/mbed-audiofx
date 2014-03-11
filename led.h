/*
 *	HAPR Project 2014
 *	Group 6 - Tom Bryant (TB) & Saul Rennison (SR)
 *
 *	File modified from TB & SR Mini-Project work
 *
 * led.c - on-board LED functions
 *
 * Defines several functions for management of on-board LED state.
 */

#ifndef _LED_H_
#define _LED_H_

#include <stdbool.h>
#include <stdint.h>

// Pass as `count` to led_blink for indefinite blinking
#define LED_BLINK_INDEFINITE 0

void led_init(void);
bool led_setup(void);
void led_set(uint8_t led, bool bState);
void led_flip(uint8_t led);
bool led_get(uint8_t led);
void led_clear(void);
void led_blink(uint32_t msec_interval, uint16_t count); // count = 0: infinite

#endif
