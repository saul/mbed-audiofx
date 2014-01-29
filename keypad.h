#ifndef _KEYPAD_H_
#define _KEYPAD_H_

#define KEYPAD_ADDR 0x21

char keypad_getc(void);
char keypad_scan(void);

#endif
