#ifndef _E_GFX_H_
#define _E_GFX_H_

#include "E.H"


extern uint8 far* BACK_BUFFER;
#define BACK_BUFFER_SEG 0xB000


void GFX_Init();
void GFX_Cleanup();
void GFX_Blit();
void GFX_LineV(int x, int y0, int y1, uint8 color);
void GFX_RectFilled(int x0, int y0, int x1, int y1, uint8 color);

#define GFX_PSet(x, y, color) {\
	BACK_BUFFER[(((y)<<8)+((y)<<6))+(x)] = (color);\
}
#define GFX_PSetWord(x, y, word) {\
	*((uint16*)(BACK_BUFFER[(((y)<<8)+((y)<<6))+(x)])) = (uint16)(word);\
}


void Palette_LoadFromFile(const char* filename);



#endif
