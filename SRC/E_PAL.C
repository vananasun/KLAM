#include "E.H"
#include "E_FILE.H"
#include <dos.h>
#include <conio.h>
#include <stdio.h>

#define PALETTE_INDEX 0x3C8
#define PALETTE_DATA  0x3C9

void Palette_LoadFromFile(const char* filename) {
	uint8 palette[768]; // 256 colours
	int i;

	FILE* f = fopen(filename, "rb");
	if (!f) Error("Palette \"%s\" was not found", filename);
	if (File_GetLength(f) != 768) {
		Error("Palette \"%s\" was of invalid size", filename);
	}

	outp(PALETTE_INDEX, 0);
	fread(palette, 768, 1, f);
	fclose(f);

	for (i = 0; i < 768; i++) {
		outp(PALETTE_DATA, palette[i]);
	}

}
