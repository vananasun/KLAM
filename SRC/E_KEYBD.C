#include "E.H"
#include "E_GFX.H"
#include "E_FONT.H"
#include "E_KEYBD.H"
#include <stdio.h>
#include <string.h>
#include <conio.h>
#include <dos.h>


#define KEY_INT 9 // ISR number
#define NUM_CODES 128

/*static const S_ASCII[NUM_CODES] = {
//  0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 0
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 1
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 2
	0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // 3
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 4
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 5
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 6
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 7
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 8
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 9
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // A
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // B
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // C
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // E
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // F
};*/
static const uint8 far S_ASCII[] = {
//   0   1   2   3   4   5   6   7   8   9   A   B   C   D   E   F
	0  ,27 ,'1','2','3','4','5','6','7','8','9','0','-','=',8  ,9  , // 0
	'q','w','e','r','t','y','u','i','o','p','[',']',13 ,0  ,'a','s', // 1
	'd','f','g','h','j','k','l',';',39 ,'`',0  ,92 ,'z','x','c','v', // 2
	'b','n','m',',','.','/',0  ,'*',0  ,' ',0  ,0  ,0  ,0  ,0  ,0  , // 3
	0  ,0  ,0  ,0  ,0  ,0  ,0  ,'7','8','9','-','4','5','6','+','1', // 4
	'2','3','0',127,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  , // 5
	0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  , // 6
	0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0	 // 7
};

static const uint8 S_ASCIISHIFT[] = {
//	 0   1   2   3   4   5   6   7   8   9   A   B   C   D   E   F
	0  ,27 ,'!','@','#','$','%','^','&','*','(',')','_','+',8  ,9  , // 0
	'Q','W','E','R','T','Y','U','I','O','P','{','}',13 ,0  ,'A','S', // 1
	'D','F','G','H','J','K','L',':',34 ,'~',0  ,'|','Z','X','C','V', // 2
	'B','N','M','<','>','?',0  ,'*',0  ,' ',0  ,0  ,0  ,0  ,0  ,0  , // 3
	0  ,0  ,0  ,0  ,0  ,0  ,0  ,'7','8','9','-','4','5','6','+','1', // 4
	'2','3','0',127,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  , // 5
	0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  , // 6
	0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0    // 7

};

static uint8 s_Keyboard[NUM_CODES];
static char s_LastASCII;
static uint8 s_LastScan, s_CurCode, s_LastCode;
static uint8 s_InInputMode = FALSE;
static void interrupt far (*s_OldKeyVect)(void);



static void clearKeyStates() {
	s_LastScan = 0;
	s_LastASCII = Key_None;
	memset(s_Keyboard, 0, sizeof(s_Keyboard));
}

static void interrupt handleKey(void) {
	static uint8 special;
	uint8 k, c, temp;
	int i;
	k = inp(0x60); // get the scan code

	// Tell the XT keyboard controller to clear the key
	outp(0x61, (temp = inp(0x61)) | 0x80);
	outp(0x61, temp);

	if (k == 0xe0) // special key prefix
		special = TRUE;
	//else if (k == 0xe1) // handle pause key
	//	s_Paused = true;
	else if (k & 0x80) { // break code
		k &= 0x7f;

		s_Keyboard[k] = 0; // off
	} else { // make code
		// @TODO: handle special (set previously bc static var)
		s_LastCode = s_CurCode;
		s_CurCode = s_LastScan = k;
		if (s_Keyboard[k] == 0)
			s_Keyboard[k] = 1; // pressed

		if (s_InInputMode) {
			if (s_Keyboard[Key_LShift] || s_Keyboard[Key_RShift]) {
				s_LastASCII = S_ASCIISHIFT[k];
			} else {
				s_LastASCII = S_ASCII[k];
			}
		}

	}

	outp(0x20, 0x20);
}


void Keyboard_Install() {
	clearKeyStates();
	s_OldKeyVect = (void*)_dos_getvect(KEY_INT);
	_dos_setvect(KEY_INT, handleKey);
}

void Keyboard_Remove() {
	//poke(0x40, 0x17, peek(0x40, 0x17) & 0xfaf0);
	uint8 __far *p = (uint8 __far *)MK_FP(0x34, 0x417);
	*p = *p & 0xfaf0;
	_dos_setvect(KEY_INT, s_OldKeyVect);
}

uint8 Keyboard_Read(uint8 scancode) {
	if (scancode >= NUM_CODES) return 0;
	if (s_Keyboard[scancode] == 1) { // advance state from pressed to held
		s_Keyboard[scancode] = 2;
		return 1;
    }
	return s_Keyboard[scancode];
}

void Keyboard_Clear(uint8 scancode) {
	s_Keyboard[scancode] = 0;
}


void Keyboard_InputString(
	int x, int y, uint8 color, char* inpStr, int maxChars)
{
	uint8 blinkTime = 0;
	char cursor = '\0';
	int len = 0;
	int maxLen = 0;
	memset(inpStr, 0, maxChars);
	s_InInputMode = TRUE;
	s_LastASCII = 0;


	for(;;) {

		if (s_LastASCII == 8) { // backspace
			if (len > 0) inpStr[--len] = '\0';
		} else if (s_LastASCII == 13) { // enter
			break;
		} else if (s_LastASCII && len < maxChars - 1) {  // regular character
			inpStr[len] = s_LastASCII;
			len++;
		}
		s_LastASCII = 0;

		// Draw text with blinking cursor
		// @TODO: Callback function
		maxLen = MAX(maxLen, len);
		GFX_RectFilled(x - 2, y - 2, 2 + x + ((2+maxLen)<<3), y + 10, 0);
		Print(x, y, color, "%s%c", inpStr, cursor);
		cursor = (blinkTime++ % 32 < 16) ?  '_' : '\0';


		GFX_Blit();
		delay(1000 / 70); // @NOTE: Maybe remove?
	}

	s_InInputMode = FALSE;
}


uint8 Keyboard_IsInInputMode() {
	return s_InInputMode;
}


void Keyboard_WaitForKey() {
	while (inp(0x60) & 0x80)
		delay(1000 / 30);
}


void Keyboard_DebugShowKeys() {
	int i;
	for (i = 0; i < sizeof(s_Keyboard); i++) {
		if (s_Keyboard[i])
			Log("Key[%i] = %i", (int)i, (int)s_Keyboard[i]);
	}
}
