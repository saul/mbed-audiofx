#ifndef _LED_H_
#define _LED_H_

#include <stdbool.h>
#include <stdint.h>

#define LED_BLINK_INDEFINITE 0

void led_init(void);
bool led_setup(void);
void led_set(uint8_t led, bool bState);
void led_flip(uint8_t led);
bool led_get(uint8_t led);
void led_clear(void);
bool led_show_bin(uint8_t v);
void led_blink(uint32_t msec_interval, uint16_t count); // count = 0: infinite

#endif
