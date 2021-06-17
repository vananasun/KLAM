#ifndef _E_KEYBD_H_
#define _E_KEYBD_H_

#include "E.H"

#define Key_None       0
#define Key_Esc        1
#define Key_Minus     12
#define Key_Equals    13
#define Key_Backspace 14
#define Key_Q         16
#define Key_W         17
#define Key_E         18
#define Key_LBracket  26
#define Key_RBracket  27
#define Key_Enter     28
#define Key_A         30
#define Key_S         31
#define Key_D         32
#define Key_LShift    42
#define Key_Z         44
#define Key_Slash     53
#define Key_RShift    54
#define Key_Spacebar  57
#define Key_Left      75
#define Key_Right     77
#define Key_Up        72
#define Key_Down      80
#define Key_Del       83

#define PRESSED 1
#define HELD 2



/**
 * Installs keyboard interrupt.
 */
void Keyboard_Install();

/**
 * Restores system's keyboard interrupt.
 */
void Keyboard_Remove();

/**
 * Gets the state of a key on the keyboard.
 *
 * @param (uint8} scancode
 *
 * @returns {uint8} state   0=off, 1=pressed, 2=held
 */
uint8 Keyboard_Read(uint8 scancode);

/**
 * Marks a given key as not pressed or held.
 *
 * @param {uint8} scancode
 */
void Keyboard_Clear(uint8 scancode);

/**
 * Waits for the user to input a string and press enter.
 *
 * @param {int}   x               - X-coord to start drawing string
 * @param {int}   y               - Y-coord to start drawing string
 * @param {uint8} color           - Color of the string
 * @param {char*} inputString     - Buffer where string is returned
 * @param {int}   inputStringSize - Size of that buffer
 */
void Keyboard_InputString(
	int x, int y, uint8 color, char* inputString, int inputStringSize
);

/**
 * Wait for a key to be pressed.
 */
void Keyboard_WaitForKey();

#endif
