#include "E_FILE.H"
#include <stdio.h>

long File_GetLength(FILE* f) {
	long size;
	if (!f) return 0;
	fseek(f, 0L, SEEK_END);
	size = ftell(f);
	fseek(f, 0L, SEEK_SET);
	return size;
}
