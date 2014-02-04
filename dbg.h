/*
 * dbg.c - Debug functions
 *
 * Defines several functions for assertions, warnings and errors (program
 * termination)
 */

#ifndef _DBG_H_
#define _DBG_H_

#include <stdio.h> // vsnprintf
#include <stdarg.h> // va_*

#define VA_FMT_STR(_fmt, _outbuf, _outbufsize) \
	{ \
		va_list args; \
		va_start(args, _fmt); \
		vsnprintf(_outbuf, _outbufsize, _fmt, args); \
		va_end(args); \
	}

#define ANSI_COLOR_RED		"\x1b[31;1m"
#define ANSI_COLOR_GREEN	"\x1b[32;1m"
#define ANSI_COLOR_YELLOW	"\x1b[33;1m"
#define ANSI_COLOR_BLUE		"\x1b[34;1m"
#define ANSI_COLOR_MAGENTA	"\x1b[35;1m"
#define ANSI_COLOR_CYAN		"\x1b[36;1m"
#define ANSI_COLOR_RESET	"\x1b[0m"
#define SGR_NO_BOLD			"\x1b[22m"
#define ANSI_CLEAR_LINE		"\x1b[1K" // erases from the current cursor position to the start of the current line (cursor does not move)

#ifdef NO_ASSERT
#	define dbg_assert(_condition, ...) (void)0
#else
#	define dbg_assert(_condition, ...) if(_condition) ; else _dbg_assert(__FILE__, __LINE__, __func__, #_condition, __VA_ARGS__)
#endif

#define dbg_error(...) _dbg_error(__FILE__, __LINE__, __func__, __VA_ARGS__)
#define dbg_warning(...) _dbg_warning(__FILE__, __LINE__, __func__, __VA_ARGS__)

void _dbg_error(const char *file, int line, const char *func, const char *format, ...)
	__attribute__ ((format (printf, 4, 5)));

void _dbg_assert(const char *file, int line, const char *func, const char *condition, const char *format, ...)
	__attribute__ ((format (printf, 5, 6)));

void _dbg_warning(const char *file, int line, const char *func, const char *format, ...)
	__attribute__ ((format (printf, 4, 5)));

void dbg_printn(const char *buf, int length);
void dbg_printf(const char *format, ...);

#endif
