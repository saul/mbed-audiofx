#ifndef _LCD_H_
#define _LCD_H_

#include <stdbool.h>
#include "lpc_types.h"

#define LCD_LINE_LEN 16
#define LCD_ADDR 0x3B
#define LCD_ASCII_OFFSET 0x80

void lcd_init(void);
uint32_t lcd_send(uint8_t *pTx, uint32_t nTxLen);
uint32_t lcd_read(uint8_t *pRx, uint32_t nRxLen);
void lcd_wait_non_busy(void);
void lcd_clear(void);
void lcd_blank(void);
void lcd_write(const char *fmt, ...) __attribute__ ((format (printf, 1, 2)));
void lcd_writec(char c);
void lcd_set_pos(uint8_t row, uint8_t col);
void lcd_blink(bool enable);
uint8_t ascii_to_lcd(char ch);

#endif
