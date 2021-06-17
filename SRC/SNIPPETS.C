#ifdef _0
#include "E.H"
#include "E_SPRITE.H"
#include "E_FILE.H"
#include "E_RENDER.H"
#include "E_PLAYER.H"
#include "E_MAP.H"
#include "E_GFX.H"
#include <dos.h>
#include <malloc.h>
#include <string.h>

#define TEX_SIZE 64
#define TRANSLUCENT_COLOR 2

typedef struct {
	uint16 width;
	uint16 height;
} SpriteFileHeader;

Sprite* g_Sprites[MAX_SPRITES];

static Image far s_Images[MAX_IMAGES];
static int s_LastImageId = 0;
static int s_ActiveSprites = 0;
static uint16 s_TexY[200]; // Sprite V offset cache

// @TODO: implement fast depth-sorting algorithm



void SpriteRenderer_RemoveInstances() {
	memset(&g_Sprites, 0, sizeof(g_Sprites));
}


void SpriteRenderer_Remove(Sprite* spr) {
	// Find index of ptr in sprite array
	int i;
	for (i = 0; i < s_ActiveSprites; i++) if (g_Sprites[i] == spr) break;

	// Shift the array back over the given sprite pointer so it will be overridden
	if (s_ActiveSprites > 1) {
		memcpy(
			&g_Sprites[i],
			&g_Sprites[i + 1],
			sizeof(Sprite) * (s_ActiveSprites - i - 1)
		);
	}

	s_ActiveSprites--;
}

void SpriteRenderer_Insert(Sprite* spr) {
	g_Sprites[s_ActiveSprites] = spr;
	s_ActiveSprites++;
}


void SpriteRenderer_RenderAll() {

	// Cache variables
	float cachedCamX    = g_Player.x;
	float cachedCamY    = g_Player.y;
	float cachedDirX    = g_Player.dirX;
	float cachedDirY    = g_Player.dirY;
	float cachedPlaneX  = g_Player.planeX;
	float cachedPlaneY  = g_Player.planeY;
	float cachedInvDet  = g_Player._invDet;
	float cachedFogDist = g_MapFogDist;
	int16 cachedSzHalfMinusScrX, cachedTexXMult;
	uint16 cachedStartYOffset, cachedPixelBufferSeg, cachedPixelBufferOff;


	uint16 y, cachedTexYSeg;



	// Other variables
	Sprite sprite;
	float sprX,sprY,sprZ, transX,transY;
	int i,x,startX,startY,endX,endY, screenX;
	int16 d, size,sizeHalf,sizeHalfMinus100, texX,texY;
	uint8 color,fogColorOffset;

	for (i = 0; i < s_ActiveSprites; i++) {

		sprite = *g_Sprites[i];


		// Translate sprite position relative to camera
		sprX = sprite.x - cachedCamX;
		sprY = sprite.y - cachedCamY;

		// Transform with inverse camera matrix
		transY = cachedInvDet * (-cachedPlaneY * sprX + cachedPlaneX * sprY);
		if (transY < 0.3f) continue; // frustum culling
		transX = cachedInvDet * (cachedDirY * sprX - cachedDirX * sprY);
		screenX = (int)(160 * (1.0f + transX / transY));

		// Calculate breadth and depth of sprite inside screen
		size = ABS((int)(200.0f / transY)); //max 266,66~ since transY>=0.75
		sizeHalf = size >> 1;
		startY = -sizeHalf + 100;
		if (startY < 0) startY = 0;
		endY = sizeHalf + 100 - 1;
		if (endY >= 200) endY = 199;

		startX = -sizeHalf + screenX;
		if (startX < 0) startX = 0;
		endX = sizeHalf + screenX;
		if (endX >= 320) endX = 319;

		// Calculate fog color offset
		fogColorOffset = (int)(transY * cachedFogDist);



		for (x = startX; x < endX; x++) {
			texX = (int)(256 * (x - (-sizeHalf + screenX)) * 64 / sizeHalf) / 256;
			if (transY > 0 && x > 0 && x < 320) { // @TODO: wallHeight check  && size < g_WallHeights[x]
				for (y = startY; y < endY; y++) {
					d = y * 256 - 200 * 128 + size * 128; // 256 and 128 factors to avoid floats
					texY = ((d * 64) / size) / 256;
					color = s_Images[sprite.imageId].pixels[s_TexY[y] + texX];
					if (TRANSLUCENT_COLOR == color) continue;
					GFX_PSet(x, y, color);
				}
			}
		}

	} // end sprite loop

}

uint16 SpriteRenderer_LoadImage(const char* filename) {
	long length;
	SpriteFileHeader h;
	FILE* f = fopen(filename, "rb");
	if (!f) {
		Error("Unable to open sprite file \"%s\"", filename);
	}

	// Check file size; First check if the header can fit at all,
	// then check whether the dimensions specified in the header correspond
	// to the file's length.
	length = File_GetLength(f);
	if (length <= sizeof(h)) { // there must atleast be one pixel
		Error("Invalid sprite file \"%s\"", filename);
		return 0;
	}
	fread(&h, sizeof(h), 1, f);
	if (length < (sizeof(h) + h.width * h.height)) {
		Error("Invalid sprite file \"%s\"", filename);
		return 0;
	}

	// Read pixels
	s_LastImageId++;
	s_Images[s_LastImageId].width = h.width;
	s_Images[s_LastImageId].height = h.height;
	s_Images[s_LastImageId].pixels = _fmalloc(h.width * h.height); // @TODO: This doesn't get free'd anywhere!
	fread(s_Images[s_LastImageId].pixels, h.width * h.height, 1, f);
	fclose(f);
	//Log("Loaded %s",filename);
	return s_LastImageId;
}


void SpriteRenderer_DrawImage(uint16 id, int startX, int startY) {
	// @TODO: Optimize with ASM
	int x, y;
	for (y = 0; y < s_Images[id].height; y++) {
		for (x = 0; x < s_Images[id].width; x++) {
			GFX_PSet(
				startX + x,
				startY + y,
				s_Images[id].pixels[s_Images[id].height * y + x]
			);
		}
	}

}

#endif
