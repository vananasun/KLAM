#include "E.H"
#include "TESTS.H"

// static void debugDrawMap() {
// 	int y, x;
// 	uint8 color;
// 	for (y = 0; y < g_MapHeight; y++) {
// 		for (x = 0; x < g_MapWidth; x++) {
// 			color = g_Map[g_MapWidth * y + x];  
// 			if (color) GFX_PSet(x, y, color);
// 		}
// 	}
// }


// uint8 isBigEndian()
// {
//     union {
//         uint32 i;
//         char c[4];
//     } bint = {0x01020304};

//     return bint.c[0] == 1; 
// }

int Tests_Perform() {
	// Log("IsBigEndian: %i", (int)isBigEndian());
	// return TRUE;
	return FALSE;
}


