#ifndef _LED_H_
#define _LED_H_

#include <stdbool.h>

void led_init(void);
int led_setup(void);
int led_set(int led, bool bState);
int led_flip(int led);
int led_get(int led);
void led_clear(void);
int led_show_bin(int v);
void led_blink(int msec_interval, int count); // count = -1: infinite

#endif
