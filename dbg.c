#include <stdio.h>

#include "dbg.h"
#include "led.h"
#include "usbcon.h"


void _dbg_error(const char *file, int line, const char *func, const char *format, ...)
{
	char buf[256];
	VA_FMT_STR(format, buf, sizeof(buf));

	// write error message
	usbcon_writef(
		ANSI_COLOR_RED "ERROR "
		ANSI_COLOR_GREEN "%s"
		ANSI_COLOR_RESET ":"
		ANSI_COLOR_GREEN "%d"
		ANSI_COLOR_RESET "@"
		ANSI_COLOR_CYAN "%s"
		ANSI_COLOR_RED ":  "
		, file, line, func);
	usbcon_write(buf, -1);
	usbcon_write(ANSI_COLOR_RESET "\r\n", -1);

	// blink LEDs infinitely if LEDs are setup
	if(led_setup())
		led_blink(200, -1);

	// halt program
	else
		while(1);
}


void _dbg_assert(const char *file, int line, const char *func, const char *condition, const char *format, ...)
{
	char buf[256];
	VA_FMT_STR(format, buf, sizeof(buf));
	_dbg_error(file, line, func, "Assertion failed: %s:  %s", condition, buf);
}


void _dbg_warning(const char *file, int line, const char *func, const char *format, ...)
{
	char buf[256];
	VA_FMT_STR(format, buf, sizeof(buf));

	usbcon_writef(
		ANSI_COLOR_YELLOW "WARNING "
		ANSI_COLOR_GREEN "%s"
		ANSI_COLOR_RESET ":"
		ANSI_COLOR_GREEN "%d"
		ANSI_COLOR_RESET "@"
		ANSI_COLOR_CYAN "%s"
		ANSI_COLOR_YELLOW ":  "
		, file, line, func);
	usbcon_write(buf, -1);
	usbcon_write(ANSI_COLOR_RESET, -1);
}

