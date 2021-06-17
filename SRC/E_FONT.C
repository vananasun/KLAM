#include "E_FONT.H"
#include "E_FILE.H"
#include "E_GFX.H"
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#define BIT_CHECK(x, pos) ((x) & (1 << (pos)))

static uint8 s_FontBuffer[2306];



uint8 Font_LoadFromFile(const char* filename) {
	int i;
	long length;
	FILE* f = fopen(filename, "rb");
	if (!f) {
		Error("Unable to open font file");
		return 0; // unable to open
	}

	length = File_GetLength(f);
	if (length != 2048 && length != 2305) {
		fclose(f);
		Error("Trying to load corrupt font");
		return 0; // corrupt file
	}


	if (length == 2048) {
		for (i = 0; i < 2048; i++) {
			fread(s_FontBuffer, 1, 2048, f); // @TODO: optimize
		}
		for (i = 0; i <= 256; i++) {
			s_FontBuffer[2048 + i] = 8;
		}
	} else {
		fread(s_FontBuffer, 1, sizeof(s_FontBuffer), f); // @TODO: optimize
	}

	fclose(f);


	// @TODO: Allocate new font "object"

	return 1;
}



void Print(int x, int y, uint8 color, const char* format, ...) {

	int i = 0, offset = 0, charX, charY;

	// Format the string
	static char internalStr[1024];
	va_list args;
	memset(internalStr, 0, 1024);

	va_start(args, format);
	vsprintf(internalStr, format, args);
	va_end(args);

	// @TODO: Calculate if will reach outside of screen and if so cancel.

	// Draw characters
	x += 8;
	while (internalStr[i] != 0) {

		offset = internalStr[i] << 3;

		for (charY = 0; charY < 8; charY++) {
			for (charX = 0; charX < 8; charX++) {
				if (!BIT_CHECK(s_FontBuffer[offset], charX)) continue;
				GFX_PSet(x - charX, y + charY, color);
			}
			offset++;
		}

		x += 8;
		i++;
	}



}

