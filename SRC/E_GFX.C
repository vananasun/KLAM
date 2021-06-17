#include "E.H"
#include "E_GFX.H"
#include <dos.h>
#include <i86.h>
#include <conio.h>
#include <string.h>

#define MODE_VGA256 0x13
#define MODE_TEXT   0x03
#define INPUT_STATUS 0x3DA

uint8 far* FRONT_BUFFER = (uint8 far*)0xA0000000L;
uint8 far* BACK_BUFFER  = (uint8 far*)0xB0000000L;

static void setVideoMode(uint8 mode) {
	union REGS regs;
	regs.h.ah = 0;
	regs.h.al = mode;
	int86(0x10, &regs, &regs);
}

void GFX_Init() {
	setVideoMode(MODE_VGA256);
	Palette_LoadFromFile("assets/palette/tunnels.pal");
	memset(BACK_BUFFER, 0, 64000);
}


void GFX_Cleanup() {
	setVideoMode(MODE_TEXT);
}

void GFX_Blit() {
	// While doing V-sync, blit the back buffer onto the video memory
	// @TODO: Don't V-Sync when lagging < 45 FPS
	while (inp(INPUT_STATUS) & 0x08);
	while (!(inp(INPUT_STATUS) & 0x08));
	memcpy(FRONT_BUFFER, BACK_BUFFER, 64000);
}



/*void GFX_PSet(int x, int y, uint8 color) {
	BACK_BUFFER[((y<<8)+(y<<6))+x] = color;
}*/

void GFX_LineV(int x, int y0, int y1, uint8 color) {
	int y;
	if (x < 0 || x >= 320) return;
	y0 = CLAMP(y0, 0, 199);
	y1 = CLAMP(y1, 0, 199);
	for (y = y0; y <= y1; y++) {
		GFX_PSet(x, y, color);
	}
}

void GFX_RectFilled(int x0, int y0, int x1, int y1, uint8 color) {
	int x, y;
	x0 = CLAMP(x0, 0, 319);
	x1 = CLAMP(x1, 0, 319);
	y0 = CLAMP(y0, 0, 199);
	y1 = CLAMP(y1, 0, 199);

	for (y = y0; y <= y1; y++) {
		for (x = x0; x <= x1; x++) {
			BACK_BUFFER[((y<<8)+(y<<6))+x] = color;
		}
	}
}
