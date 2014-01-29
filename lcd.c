#include <string.h> // strlen

#include "i2c.h"
#include "lcd.h"
#include "usbcon.h"
#include "dbg.h"


// Reset sequence from p21 http://www-module.cs.york.ac.uk/hapr/resources/mbed_resources/datasheets/batron_operating_instructions_312175.pdf
void lcd_init(void)
{
	usbcon_writef("Initialising LCD... ");

	uint8_t buf[] = {
		0x00, // NOP

		// Function set (normal)
		0x34, // D4 = Data Len (1 for 8 bit), D2 = Num Lines (1 for 2x16), D1 = MUX (0 for 2x16/1x32), D0 = Instruction set (0 for normal)

		// Display control
		0x0C, // D2 = Display Ctrl (1 for on), D1 = Cursor ctrl (0 for off), D0 = Cursor Blink (0 for off)

		// Entry mode set
		0x06, // D1 = Cursor Dir (1 for inc), D0 = shift (0 for off)

		// Cursor/display shift
		0x14, // Move cursor, right shift

		// Function set (extended)
		0x35, // D4 = Data Len (1 for 8 bit), D2 = Num Lines (1 for 2x16), D1 = MUX (0 for 2x16/1x32), D0 = Instruction set (1 for extended)

		// Display configuration
		0x04, // D1 = Col data (0 for L to R), D0 = row data (0 for Top to Bot)

		// Temperature control
		0x10, // D1 = TC1, D2=TC2 (colour temperature)

		// Set VLCD
		0x9F, // D5:D0 = voltage, D6 = V register (0 for VA)

		// Function set (normal)
		0x34, // D4 = Data Len (1 for 8 bit), D2 = Num Lines (1 for 2x16), D1 = MUX (0 for 2x16/1x32), D0 = Instruction set (0 for normal)

		// Set DDRAM address
		0x80, // DRAM to 0x00

		// Return home
		0x02
	};

	i2c_transfer(LCD_ADDR, buf, sizeof(buf), NULL, 0);

	lcd_blank();

	usbcon_writef(ANSI_COLOR_GREEN "OK!\r\n" ANSI_COLOR_RESET);
}


uint32_t lcd_send(uint8_t *pTx, uint32_t nTxLen)
{
	lcd_wait_non_busy();

	//i2c_debug(1);
	uint32_t status = i2c_transfer(LCD_ADDR, pTx, nTxLen, NULL, 0);
	//i2c_debug(0);

	return status;
}


uint32_t lcd_read(uint8_t *pRx, uint32_t nRxLen)
{
	uint8_t tx = 0x00;

	//i2c_debug(1);
	uint32_t status = i2c_transfer(LCD_ADDR, &tx, 1, pRx, nRxLen);
	//i2c_debug(0);

	return status;
}


void lcd_wait_non_busy(void)
{
	uint8_t rxb = 0;
	do
	{
		lcd_read(&rxb, 1);
	} while(rxb & 0x80);
}


void lcd_clear(void)
{
	uint8_t buf[] = {0x00, 0x01};
	lcd_send(buf, sizeof(buf));
}


void lcd_blank(void)
{
	lcd_set_pos(0, 0);
	lcd_write("                ");
	lcd_set_pos(1, 0);
	lcd_write("                ");
	lcd_set_pos(0, 0);
}


void lcd_write(const char *fmt, ...)
{
	char text[32];
	VA_FMT_STR(fmt, text, sizeof(text));

	uint8_t buf[1 + LCD_LINE_LEN];
	buf[0] = 0x40;

	int nChars = strlen(text);

	if(nChars > LCD_LINE_LEN)
	{
		dbg_warning("Buffer overflow. Tried to write %d (or more) chars to LCD (max %d)\r\n", nChars, LCD_LINE_LEN);
		nChars = LCD_LINE_LEN;
	}

	for(int i = 0; i < nChars; i++)
		buf[i+1] = ascii_to_lcd(text[i]);

	lcd_send(buf, 1 + nChars);
}


void lcd_writec(char c)
{
	uint8_t buf[] = {0x40, ascii_to_lcd(c)};
	lcd_send(buf, sizeof(buf));
}


void lcd_set_pos(int row, int col)
{
	dbg_assert(row >= 0 && row <= 1, "invalid row");
	dbg_assert(col >= 0 && col < 16, "invalid column");

	uint8_t buf[] = {
		0x00,
		0x80 + (row * 0x40) + col
	};

	lcd_send(buf, sizeof(buf));
}


void lcd_blink(int enable)
{
	uint8_t buf[] = {0x00, 0x0C | enable};
	lcd_send(buf, sizeof(buf));
}


uint8_t ascii_to_lcd(char ch)
{
	char c = 0xE0; // default: upside down ?

	if((ch >= ' ' && ch <= '?' ) ||
		(ch >= 'A' && ch <= 'Z') ||
		(ch >= 'a' && ch <= 'z'))
		c = LCD_ASCII_OFFSET + ch;

	return c;
}

