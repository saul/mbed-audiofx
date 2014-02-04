/*
 * dbg.c - Debug functions
 *
 * Defines several functions for assertions, warnings and errors (program
 * termination)
 */

#include <stdio.h>
#include <string.h>

#include "dbg.h"
#include "led.h"
#include "packets.h"


/*
 * _dbg_error (use macro dbg_error)
 *
 * Prints an error in red to the console and halts program execution.
 *
 * If the LED subsystem has been setup, the LEDs will blink indefinitely.
 */
void _dbg_error(const char *file, int line, const char *func, const char *format, ...)
{
	char buf[256];
	VA_FMT_STR(format, buf, sizeof(buf));

	// write error message
	dbg_printf(
		ANSI_COLOR_RED "ERROR "
		ANSI_COLOR_GREEN "%s"
		ANSI_COLOR_RESET ":"
		ANSI_COLOR_GREEN "%d"
		ANSI_COLOR_RESET "@"
		ANSI_COLOR_CYAN "%s"
		ANSI_COLOR_RED ":  "
		, file, line, func);
	dbg_printn(buf, -1);
	dbg_printn(ANSI_COLOR_RESET "\r\n", -1);

	// blink LEDs infinitely if LEDs are setup
	if(led_setup())
		led_blink(200, -1);

	// halt program
	else
		while(1);
}


/*
 * _dbg_assert (use macro dbg_assert)
 *
 * If condition is false, error and halt program execution.
 */
void _dbg_assert(const char *file, int line, const char *func, const char *condition, const char *format, ...)
{
	char buf[256];
	VA_FMT_STR(format, buf, sizeof(buf));
	_dbg_error(file, line, func, "Assertion failed: %s:  %s", condition, buf);
}


/*
 * _dbg_warning (use macro dbg_warning)
 *
 * Print a warning in yellow to console.
 */
void _dbg_warning(const char *file, int line, const char *func, const char *format, ...)
{
	char buf[256];
	VA_FMT_STR(format, buf, sizeof(buf));

	dbg_printf(
		ANSI_COLOR_YELLOW "WARNING "
		ANSI_COLOR_GREEN "%s"
		ANSI_COLOR_RESET ":"
		ANSI_COLOR_GREEN "%d"
		ANSI_COLOR_RESET "@"
		ANSI_COLOR_CYAN "%s"
		ANSI_COLOR_YELLOW ":  "
		, file, line, func);
	dbg_printn(buf, -1);
	dbg_printn(ANSI_COLOR_RESET, -1);
}


/*
 * dbg_printn
 *
 * Writes a string to the console. If length < 0, the number of characters to
 * write is taken as the length of `buf`.
 */
void dbg_printn(const char *buf, int length)
{
	if(length < 0)
		length = strlen(buf);

	packet_print_send(buf, length);
}


/*
 * dbg_printf
 *
 * Writes a formatted string to the connected machine's console.
 */
void dbg_printf(const char *format, ...)
{
	va_list args;
	va_start(args, format);

	char buf[128];
	vsnprintf(buf, sizeof(buf), format, args);

	va_end(args);

	dbg_printn(buf, -1);
}
