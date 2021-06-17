#ifndef __E_MAP_H__
#define __E_MAP_H__

typedef struct {
	uint8 version; // engine version
	uint16 width;
	uint16 height;
	uint8 ceilColor;
	uint8 floorColor;
	uint8 snowing;
	uint16 instanceCount;
	float fogDistance;
	uint16 spawnX;
	uint16 spawnY;
} MapFileHeader;

//typedef struct {
//
//} MapFileInstance;


// @TODO: Put this in a struct
extern float g_MapFogDist;
extern uint16 g_MapWidth;
extern uint16 g_MapHeight;
extern uint8 g_MapCeilColor;
extern uint8 g_MapFloorColor;
extern uint8 g_MapSnowing;
extern uint16 g_MapSpawnX;
extern uint16 g_MapSpawnY;
extern uint8 far *g_Map;

uint8 MapManager_LoadFromFile(const char* filename);
uint8 MapManager_SaveToFile(const char* filename);
void MapManager_Cleanup();
void MapManager_Resize(uint16 width, uint16 height);

#endif
