#include "E.H"
#include "E_MAP.H"
#include "E_PART.H"
#include "E_PLAYER.H"
#include "E_SPRITE.H"
#include "E_ENTITY.H"
#include "G_GAME.H"
#include <dos.h>
#include <stdio.h>
#include <malloc.h>
#include <string.h>

#include "E_GFX.H"


static uint8 s_MapVersion = 0;
float g_MapFogDist = 4;
uint16 g_MapWidth = 16;
uint16 g_MapHeight = 16;
uint8 g_MapCeilColor = 0;//11;
uint8 g_MapFloorColor = 0;//54;
uint8 g_MapSnowing = FALSE;
uint16 g_MapSpawnX;
uint16 g_MapSpawnY;
uint8* far g_Map = 0;



static void readHeader(FILE* f) {
	MapFileHeader header;
	fread(&header, sizeof(header), 1, f);
	g_MapWidth = header.width;
	g_MapHeight = header.height;
	g_MapCeilColor = header.ceilColor;
	g_MapFloorColor = header.floorColor;
	g_MapSnowing = header.snowing;
	g_MapFogDist = header.fogDistance;

	switch (header.version) {
	case 0:
		s_MapVersion = 1;
		g_MapSpawnX = 2;
		g_MapSpawnY = 2;
		break;
	case 1:
		break;
	case 2:
		break;
	default:
		break;
	}
}

static void readPlayer(FILE* f) {
	// @TODO: Check if it is a save file
	UNUSED_PARAMETER(f); //fread();
}

static void writeHeader(FILE* f) {
	MapFileHeader header;
	header.version = s_MapVersion;
	header.width = g_MapWidth;
	header.height = g_MapHeight;
	header.ceilColor = g_MapCeilColor;
	header.floorColor = g_MapFloorColor;
	header.snowing = FALSE;
	header.instanceCount = 0;
	header.fogDistance = g_MapFogDist;
	header.spawnX = g_MapSpawnX;
	header.spawnY = g_MapSpawnY;
	fwrite(&header, sizeof(header), 1, f);
}

static void writePlayer(FILE* f) {
	// @TODO: Write player
	fwrite(&g_Player.x, 2, 1, f);
	fwrite(&g_Player.y, 2, 1, f);
}


static void writeEntities(FILE* f) {
	Entity* e;
	uint16 i;
	fwrite(&g_EntityCount, 2, 1, f);
	for (i = 0; i < g_EntityCount; i++) {
		e = &g_Entities[i];
		fwrite(e, sizeof(Entity), 1, f);
		fwrite(e->_, e->_size, 1, f);

		// Write de-referenced sprite struct
		if (e->sprite) {
			fwrite(e->sprite, sizeof(Sprite), 1, f);
		}

	}
}

static void readEntities(FILE* f) {
	uint16 i, entityCount;
	Entity e;
	Sprite* spr;
	fread(&entityCount, 2, 1, f);

	for (i = 0; i < entityCount; i++) {
		fread(&e, sizeof(Entity), 1, f);
		e._ = malloc(e._size); // @TODO: free aswell
		fread(e._, e._size, 1, f);

		// Read sprite
		if (e.sprite != 0) { // It had a pointer upon writing
			spr = (Sprite*)malloc(sizeof(Sprite));
			fread(spr, sizeof(Sprite), 1, f);
			e.sprite = spr;
		}

		Game_InitEntity(e);

	}
}




uint8 MapManager_LoadFromFile(const char* filename) {
	FILE* f = fopen(filename, "rb");
	if (!f) return FALSE;
	readHeader(f);

	Entities_Reset();
	ParticleManager_Reset();
	SpriteRenderer_RemoveInstances();

	// (Re-)allocate and read walls
	if (!g_Map) g_Map = _fmalloc(g_MapWidth * g_MapHeight);
	fread(g_Map, g_MapWidth * g_MapHeight, 1, f);

	readEntities(f);
	readPlayer(f);
	if (f) fclose(f);

	return TRUE;
}

uint8 MapManager_SaveToFile(const char* filename) {
	// MapFileInstance instance;

	// Open file and write header
	FILE* f = fopen(filename, "wb");
	if (!f) return FALSE;
	writeHeader(f);


	fwrite(g_Map, g_MapWidth * g_MapHeight, 1, f);
	writeEntities(f);
	writePlayer(f);


	// Finish up writing
	if (f) fclose(f);
	return TRUE;

}

void MapManager_Cleanup() {
	if (g_Map) free(g_Map);
}

void MapManager_Resize(uint16 width, uint16 height) {

	// Create temporary resized version of map
	uint16 x, y;
	uint8* tmpMap = malloc(width * height);
	for (y = 0; y < height; y++) {
		for (x = 0; x < width; x++) {
			tmpMap[width * y + x] = g_Map[g_MapWidth * y + x];
		}
	}

	// Ensure that walls persist around the edges
	for (x = 0; x < width; x++) {
		if (!tmpMap[width * (height - 1) + x])
			tmpMap[width * (height - 1) + x] = 1;
	}
	for (y = 0; y < height; y++) {
		if (!tmpMap[width * y + (width - 1)])
			tmpMap[width * y + (width - 1)] = 1;
	}

	// Copy temporary resized map into actual map buffer
	_ffree(g_Map);
	g_Map = _fmalloc(width * height);
	memcpy(g_Map, tmpMap, width * height);
	free(tmpMap);


	g_MapWidth = width;
	g_MapHeight = height;
}
