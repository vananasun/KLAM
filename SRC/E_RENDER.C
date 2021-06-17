#include "E.H"
#include "E_RENDER.H"
#include "E_PLAYER.H"
#include "E_MAP.H"
#include "E_GFX.H"
#include <dos.h>

int g_WallHeights[320];
static float s_CameraX[320];
static uint16 s_CachedCeilColor;  // uint16 because these
static uint16 s_CachedFloorColor; // will be in CPU registers




static void drawCol(int x, int y0, int y1, uint16 colorReg, uint16 ceilColor, uint16 floorColor) {

	__asm {
	// set segment to video back buffer
	mov  bx, BACK_BUFFER_SEG
	mov  es, bx

	mov  di, [x] // start drawing at coords (x,0)


	// draw until di = 320 * y0 + x
	mov  cx, [y0]
	cmp  cx, 0
	jle  afterCeilLoop
	mov  bx, cx
	shl  bx, 6
	shl  cx, 8
	add  cx, bx
	add  cx, di // += x

	mov  ax, [ceilColor]


ceilLoop:
	mov  es:[di], ax
	add  di, 320
	cmp  di, cx
	jl   ceilLoop
afterCeilLoop:


	// Stop drawing at 320 * y1 + x
	mov  cx, [y1]
	mov  bx, cx
	shl  cx, 8
	shl  bx, 6
	add  cx, bx
	add  cx, 320 // fix bottom not visible

	mov  ax, [colorReg]
wallLoop:
	mov  es:[di], ax
	add  di, 320
	cmp  di, cx
	jbe  wallLoop


	// Draw until di >= 320 * 200 + x
	mov  cx, 320 * 200
	cmp  di, cx
	jge  afterFloorLoop

	mov  ax, [floorColor]
floorLoop:
	mov  es:[di], ax
	add  di, 320
	cmp  di, cx
	jbe  wallLoop
afterFloorLoop:
	}

}


static void drawEmptyColumn(int x) {

	// int y = 0;
	// do {
	// 	GFX_PSet(x, y, s_CachedCeilColor);
	// } while (++y < 99);
	// do {
	// 	GFX_PSet(x, y, s_CachedFloorColor);
	// } while (++y < 199);

	__asm {
	// Set segment to video buffer
	mov  bx, BACK_BUFFER_SEG
	mov  es, bx

	mov  di, [x] // start drawing at coords (x,0)
	mov  cx, di
	add  cx, 32000 // draw until end of screen
	mov  ax, [s_CachedCeilColor]

ceilLoop:
	mov  es:[di], ax
	add  di, 320
	cmp  di, cx
	jb   ceilLoop


	add  cx, 32000
	mov  ax, [s_CachedFloorColor]
floorLoop:
	mov  es:[di], ax
	add  di, 320
	cmp  di, cx
	jb   floorLoop
	}

}


static void genCameraXTable() {
	int x = 0;
	for (x = 0; x < 320; x += 2) {
		s_CameraX[x] = 2 * x / (float)320 - 1;
	}
}



/***
 *
 * Public methods
 *
 ***/

void Renderer_Init() {
	genCameraXTable();
	Renderer_UpdateCache();
}

void Renderer_Cleanup() {

}

void Renderer_UpdateCache() {
	s_CachedCeilColor = g_MapCeilColor | (g_MapCeilColor << 8);
	s_CachedFloorColor = g_MapFloorColor | (g_MapFloorColor << 8);
}

void Renderer_RenderWalls() {

	//
	uint8 color, wallColor, side;
	int x, mapX, mapY, stepX, stepY, lineHeight, drawStart, drawEnd;
	float cameraX, rayDirX, rayDirY, sideDistX, sideDistY,
		  deltaDistX, deltaDistY, perpWallDist;

	// Cached variables
	float cachedFogDist    = g_MapFogDist;
	float cachedCamX       = g_Player.x;
	float cachedCamY       = g_Player.y;
	float cachedDirX       = g_Player.dirX;
	float cachedDirY       = g_Player.dirY;
	float cachedPlaneX     = g_Player.planeX;
	float cachedPlaneY     = g_Player.planeY;
	float cachedCamRelXNeg = cachedCamX - (int)cachedCamX;
	float cachedCamRelXPos = (int)cachedCamX + 1.0f - cachedCamX;
	float cachedCamRelYNeg = cachedCamY - (int)cachedCamY;
	float cachedCamRelYPos = (int)cachedCamY + 1.0f - cachedCamY;

	// Log("ceil: %i, floor: %i", (int)g_MapCeilColor, (int)g_MapFloorColor);

	for (x = 0; x < 320; x += 2) {

		// Calculate ray position and direction
		cameraX = s_CameraX[x]; // x-coord in camera space
		rayDirX = cachedDirX + cachedPlaneX * cameraX;
		rayDirY = cachedDirY + cachedPlaneY * cameraX;

		// Which box of the map we're in
		mapX = (int)cachedCamX;
		mapY = (int)cachedCamY;

		// Length of ray from one x or y-side to next x or y-side
		deltaDistX = (0.0f == rayDirX) ? 1.0f : ABS(1.0f / rayDirX);
		deltaDistY = (0.0f == rayDirY) ? 1.0f : ABS(1.0f / rayDirY);

		// Calculate step and initial sideDist and then perform DDA.
		// This is an unrolled version of a single DDA loop into 4 loops.
		// Each loop has it's own direction to walk in.
		if (rayDirX < 0) {
			sideDistX = cachedCamRelXNeg * deltaDistX;
			if (rayDirY < 0) {
				sideDistY = cachedCamRelYNeg * deltaDistY;
				for(;;) {
					if (sideDistX < sideDistY) {
						sideDistX += deltaDistX;
						if (g_Map[g_MapWidth * mapY + --mapX]) {
							perpWallDist = (mapX - cachedCamX + 1) / rayDirX;
							side = 0;
							break;
						}
					} else {
						sideDistY += deltaDistY;
						if (g_Map[g_MapWidth * --mapY + mapX]) {
							perpWallDist = (mapY - cachedCamY + 1) / rayDirY;
							side = 1;
							break;
						}
					}
				}
			} else {
				sideDistY = cachedCamRelYPos * deltaDistY;
				for(;;) {
					if (sideDistX < sideDistY) {
						sideDistX += deltaDistX;
						if (g_Map[g_MapWidth * mapY + --mapX]) {
							perpWallDist = (mapX - cachedCamX + 1) / rayDirX;
							side = 0;
							break;
						}
					} else {
						sideDistY += deltaDistY;
						if (g_Map[g_MapWidth * ++mapY + mapX]) {
							perpWallDist = (mapY - cachedCamY) / rayDirY;
							side = 1;
							break;
						}
					}
				}
			}
		} else {
			sideDistX = cachedCamRelXPos * deltaDistX;
			if (rayDirY < 0) {
				sideDistY = cachedCamRelYNeg * deltaDistY;
				for(;;) {
					if (sideDistX < sideDistY) {
						sideDistX += deltaDistX;
						if (g_Map[g_MapWidth * mapY + ++mapX]) {
							side = 0;
							perpWallDist = (mapX - cachedCamX) / rayDirX;
							break;
						}
					} else {
						sideDistY += deltaDistY;
						if (g_Map[g_MapWidth * --mapY + mapX]) {
							perpWallDist = (mapY - cachedCamY + 1) / rayDirY;
							side = 1;
							break;
						}
					}
				}
			} else {
				sideDistY = cachedCamRelYPos * deltaDistY;
				for(;;) {
					if (sideDistX < sideDistY) {
						sideDistX += deltaDistX;
						if (g_Map[g_MapWidth * mapY + ++mapX]) {
							side = 0;
							perpWallDist = (mapX - cachedCamX) / rayDirX;
							break;
						}
					} else {
						sideDistY += deltaDistY;
						if (g_Map[g_MapWidth * ++mapY + mapX]) {
							perpWallDist = (mapY - cachedCamY) / rayDirY;
							side = 1;
							break;
						}
					}
				}
			}
		}

		// Calculate height of line to draw on screen
		// 200 / 32768 = 0.006103515625 is the max value before perpWallDist
		// will glitch out due to 16-bit int overflow.
		// @TODO: Make use of x86 CPU overflow (or carry if unsigned) flag
		//        instead of expensive x87 float comparison.
		if (perpWallDist <= 0.006103515625)
			lineHeight = 200;
		else
			lineHeight = (int)(200.0f / perpWallDist);

		// Store wall depth for later usage
		//g_ZBuffer[x    ] = perpWallDist;
		//g_ZBuffer[x + 1] = g_ZBuffer[x];
		g_WallHeights[x    ] = lineHeight;
		g_WallHeights[x + 1] = lineHeight;

		// For walls that are too far, render an empty column
		if (lineHeight <= 3) { // @TODO: lower this and fix distance thing
			drawEmptyColumn(x);
			continue;
		}

		// Calculate lowest and highest pixel to fill in current stripe
		drawStart = -lineHeight / 2 + 100;
		if (drawStart < 0) drawStart = 0;
		drawEnd = lineHeight / 2 + 100;
		if (drawEnd >= 200) drawEnd = 199;

		// Choose wall color; First we calculate the base color.
		// Wall colours in the palette are stored starting from the second
		// palette row with 16 columns (shades) of each.
		//
		// After we picked the base color, we will calculate it's shade from
		// the wall distance. If it is too far away, we use the fog colour.
		//
		//
		// @TODO: Optimize this further later on when I'm not sleepy!
		// @TODO: Use shiny wall colours
		// @TODO: Use bright colours aswell that don't get shown due to them
		//        being to close to the camera.
		//
		wallColor = (g_Map[g_MapWidth * mapY + mapX] << 4);
		perpWallDist = MAX(0.0f, perpWallDist - 1.0f); // @NOTE: TEMPORARY BRIGHT COLOUR FIX
		color = MIN(
			wallColor + (int)((side?(10.0f/16.0f):1.0f) * perpWallDist * cachedFogDist) + (side?6:0),
			wallColor + 16
		);
		if (color < wallColor)
			color = wallColor;
		/*
		color = MIN(
			wallColor + (int)((side?(10.0f/16.0f):1.0f) * perpWallDist * cachedFogDist) + (side?4:-2),
			wallColor + 16
		);
		if (color < wallColor)
			color = wallColor;
		*/
		color = (16 + wallColor == color) ? 15 : color;

		//Log("x %i y0 %i y1 %i c %i", drawStart,drawEnd,(int)color);
		// Draw the pixels of the stripe as a vertical line
		drawCol(x, drawStart, drawEnd, color | (color << 8), s_CachedCeilColor, s_CachedFloorColor);

	}

}

