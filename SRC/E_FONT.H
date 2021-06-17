#ifndef __E_FONT_H__
#define __E_FONT_H__
#include "E.H"

uint8 Font_LoadFromFile(const char* filename);
void Print(int x, int y, uint8 color, const char* format, ...);

#endif
