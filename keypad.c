#include "i2c.h"
#include "dbg.h"
#include "keypad.h"


typedef struct KeyState_t
{
	int bKeyDown;
} KeyState_t;

typedef struct KeypadState_t
{
	char chLast;
	KeyState_t ppKeyState[4][4]; // key states
} KeypadState_t;

static KeypadState_t s_KeypadState;
static char s_chKeyMap[4][4] = {{'D', '#', '0', '*'}, {'C', '9', '8', '7'}, {'B', '6', '5', '4'}, {'A', '3', '2', '1'}};


/*
 * keypad_init
 *
 * Resets keypad key press states.
 */
void keypad_init(void)
{
	s_KeypadState.chLast = 0;

	for(int row = 0; row < 4; ++row)
	{
		for(int col = 0; col < 4; ++col)
		{
			s_KeypadState.ppKeyState[row][col].bKeyDown = 0;
		}
	}
}


/*
 * keypad_keydown_count
 *
 * @returns number of keys pressed
 */
int keypad_keydown_count(void)
{
	int nKeysDown = 0;

	for(int row = 0; row < 4; ++row)
	{
		for(int col = 0; col < 4; ++col)
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
int keypad_is_keydown(char key)
{
	// Convert key -> row/col
	int row, col;
	for(row = 0; row < 4; ++row)
	{
		for(col = 0; col < 4; ++col)
		{
			if(s_chKeyMap[row][col] == key)
				break;
		}
	}

	if(row == 4 || col == 4)
	{
		dbg_error("key %c is not valid (not on keypad)", key);
		return -1;
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
	for(int row = 0; row < 4; ++row)
	{
		for(int col = 0; col < 4; ++col)
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
 * Transfer a byte to keypad via I2C. Asserts on transfer failure.
 *
 * @returns byte returned from device
 */
static uint8_t keypad_transfer(uint8_t tx)
{
	uint8_t rx = 0;
	dbg_assert(i2c_transfer(KEYPAD_ADDR, &tx, 1, &rx, 1) == SUCCESS, "keypad transfer failed");
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
	int iKeysDown = keypad_keydown_count();
	char chKeyPressed = 0;

	for(int col = 0; col < 4; ++col)
	{
		// Set bit of column we want to read in upper nibble, then invert
		// (Column scanning is active low)
		int tx = ~(1 << (4+col));

		// Read inverted lower nibble of returned value
		// (Keys down for this column are active low in the lower nibble)
		int rx = (~keypad_transfer(tx)) & 0x0F;

		// Test each row
		for(int row = 0; row < 4; ++row)
		{
			// Key down
			if(rx & (1 << row))
			{
				// Key was already down
				if(s_KeypadState.ppKeyState[row][col].bKeyDown)
					continue;

				s_KeypadState.ppKeyState[row][col].bKeyDown = 1;
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

				s_KeypadState.ppKeyState[row][col].bKeyDown = 0;
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
