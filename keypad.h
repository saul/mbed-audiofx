/*
 * keypad.c - i2c keypad functions
 *
 * Defines several functions for testing and managing which keys are pressed
 * on the keypad. Handles multiple key presses.
 */

#ifndef _KEYPAD_H_
#define _KEYPAD_H_

// i2c address of keypad
#define KEYPAD_ADDR 0x21


char keypad_getc(void);
char keypad_scan(void);
bool keypad_is_keydown(char key);

#endif
