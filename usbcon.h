/*
 * usbcon.c - USB console support
 *
 * Defines several functions for very basic terminal emulation via USB.
 *
 * To interact with the console, use `screen /dev/ttyACM0`
 */

#ifndef _USBCON_H_
#define _USBCON_H_

void usbcon_init(void);
int usbcon_write(const char *buf, int length);
int usbcon_writef(const char *format, ...) __attribute__ ((format (printf, 1, 2)));
int usbcon_read(char *buf, int length);
const char *usbcon_readline(void);

#endif
