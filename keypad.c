/*
 * keypad.c - i2c keypad functions
 *
 * Defines several functions for testing and managing which keys are pressed
 * on the keypad. Handles multiple key presses.
 */

#include "i2c.h"
#include "dbg.h"
#include "keypad.h"


typedef struct
{
	bool bKeyDown;
} KeyState_t;

typedef struct
{
	char chLast;
	KeyState_t ppKeyState[4][4]; // key states
} KeypadState_t;

// Global keypad keypress state
static KeypadState_t s_KeypadState;

// Map from row/col -> ASCII character
static char s_chKeyMap[4][4] = {{'D', '#', '0', '*'}, {'C', '9', '8', '7'}, {'B', '6', '5', '4'}, {'A', '3', '2', '1'}};


/*
 * keypad_init
 *
 * Resets keypad key press states.
 */
void keypad_init(void)
{
	s_KeypadState.chLast = 0;

	for(uint8_t row = 0; row < 4; ++row)
	{
		for(uint8_t col = 0; col < 4; ++col)
		{
			s_KeypadState.ppKeyState[row][col].bKeyDown = false;
		}
	}
}


/*
 * keypad_keydown_count
 *
 * @returns number of keys pressed
 */
 uint8_t keypad_keydown_count(void)
{
	uint8_t nKeysDown = 0;

	for(uint8_t row = 0; row < 4; ++row)
	{
		for(uint8_t col = 0; col < 4; ++col)
		{
			if(s_KeypadState.ppKeyState[row][col].bKeyDown)
				nKeysDown++;
		}
	}

	return nKeysDown;
}


/*
 * keypad_is_keydown
 *
 * @returns is key `key` pressed?
 */
bool keypad_is_keydown(char key)
{
	// Convert key -> row/col
	bool bFound = false;
	uint8_t row, col;

	for(row = 0; row < 4; ++row)
	{
		for(col = 0; col < 4; ++col)
		{
			if(s_chKeyMap[row][col] != key)
				continue;

			bFound = true;
			break;
		}

		if(bFound)
			break;
	}

	if(row == 4 || col == 4)
	{
		dbg_error("key %c is not valid (not on keypad)", key);
		return false;
	}

	return s_KeypadState.ppKeyState[row][col].bKeyDown;
}


/*
 * keypad_get_keydown
 *
 * Get character of current key press (does not handle multiple key presses)
 *
 * @returns ASCII character of key
 */
static char keypad_get_keydown(void)
{
	for(uint8_t row = 0; row < 4; ++row)
	{
		for(uint8_t col = 0; col < 4; ++col)
		{
			if(s_KeypadState.ppKeyState[row][col].bKeyDown)
				return s_chKeyMap[row][col];
		}
	}

	return 0;
}


/*
 * keypad_transfer
 *
 * Transfer a byte to keypad via I2C.
 *
 * @returns byte returned from device
 */
static uint8_t keypad_transfer(uint8_t tx)
{
	uint8_t rx = 0;
	i2c_transfer(KEYPAD_ADDR, &tx, 1, &rx, 1);
	return rx;
}


/*
 * keypad_getc
 *
 * Read a single keypress from the key pad. Returns when physical key is
 * pressed.
 *
 * @returns ASCII character of key press
 */
char keypad_getc(void)
{
	dbg_printf("keypad_getc: ");

	// Loop until we key a non NULL key press
	char key;
	while(!(key = keypad_scan()))
		;

	dbg_printf("`%c`\r\n", key);
	return key;
}


/*
 * keypad_scan
 *
 * Update keypad button states. If a key is pressed (on its own), then that key
 * is returned. If multiple keys were pressed, but now only one is down, return
 * the remaining key.
 *
 * @returns key character when key has been pressed since last call, otherwise 0
 */
char keypad_scan(void)
{
	uint8_t iKeysDown = keypad_keydown_count();
	char chKeyPressed = 0;

	for(uint8_t col = 0; col < 4; ++col)
	{
		// Set bit of column we want to read in upper nibble, then invert
		// (Column scanning is active low)
		uint8_t tx = ~(1 << (4+col));

		// Read inverted lower nibble of returned value
		// (Keys down for this column are active low in the lower nibble)
		uint8_t rx = (~keypad_transfer(tx)) & 0x0F;

		// Test each row
		for(uint8_t row = 0; row < 4; ++row)
		{
			// Key down
			if(rx & (1 << row))
			{
				// Key was already down
				if(s_KeypadState.ppKeyState[row][col].bKeyDown)
					continue;

				s_KeypadState.ppKeyState[row][col].bKeyDown = true;
				iKeysDown++;

				// If no other keys are down, return this key
				if(iKeysDown == 1)
					chKeyPressed = s_chKeyMap[row][col];
			}

			// Key up
			else
			{
				// Key was already up
				if(!s_KeypadState.ppKeyState[row][col].bKeyDown)
					continue;

				s_KeypadState.ppKeyState[row][col].bKeyDown = false;
				iKeysDown--;

				char chKeyDown = keypad_get_keydown();

				// If 2 keys were down (now 1), return the other key that's down
				if(iKeysDown == 1 && chKeyDown != s_KeypadState.chLast)
					chKeyPressed = chKeyDown;
			}
		}
	}

	dbg_assert(keypad_keydown_count() == iKeysDown, "key down count mismatch");

	// If no keys are down reset the last key press. If a key is pressed, set
	// that as the last press
	if(chKeyPressed || iKeysDown == 0)
		s_KeypadState.chLast = chKeyPressed;

	return chKeyPressed;
}
