#include "E.H"
#include "E_EDITOR.H"
#include "E_PLAYER.H"
#include "E_KEYBD.H"
#include "E_MAP.H"
#include "E_SPRITE.H"
#include "E_GFX.H"
#include "E_ENTITY.H"
#include "G_GAME.H"
#include <stdlib.h>
#include <conio.h>

#define CURSOR_MOVE_RATE 3
#define CURSOR_MOVE_DELAY 50

#define COLOR_CURSOR (1 + rand() % 255)
#define COLOR_ENTITY 12


uint8 g_EditorActive = FALSE;
static int s_CursorX;
static int s_CursorY;
static uint8 s_CursorTimeout = 0; // Repeat delay when moving cursor
static uint8 s_CursorMoved = FALSE;
static uint8 s_CursorMovedPrev = FALSE;
static uint8 s_CursorWallType = 1;
static uint8 s_PlayerFlash = 0;
static int s_CursorClass = ENTITY_PAAL;



static void dialogSaveMap() {
	char filename[64];
	Keyboard_InputString(2, 2, 9, filename, sizeof(filename));
	MapManager_SaveToFile(filename);
}

static void dialogLoadMap() {
	char filename[64];
	Keyboard_InputString(2, 2, 9, filename, sizeof(filename));
	if (!MapManager_LoadFromFile(filename))
		return; // @TODO: handle nonexistant file

	s_CursorX = g_MapWidth / 2;
	s_CursorY = g_MapHeight / 2;
}




static void drawMapPreview() {
	int x,y, color;
	for (y = 0; y < g_MapHeight; y++) {
		for (x = 0; x < g_MapWidth; x++) {
			color = g_Map[g_MapWidth * y + x];
			color = color ? (color << 4) : 0;
			GFX_PSet(2*x  , 2*y  , color);
			GFX_PSet(2*x+1, 2*y  , color);
			GFX_PSet(2*x  , 2*y+1, color);
			GFX_PSet(2*x+1, 2*y+1, color);
		}
	}
}

static void tryMoveCursor(uint8 scancode, int8 xRel, int8 yRel) {
	uint8 state = Keyboard_Read(scancode);
	if (1 == state && !s_CursorMovedPrev) {
		if ((s_CursorTimeout == 0) || s_CursorMoved) {
			s_CursorX = WRAP(s_CursorX + xRel, 0, g_MapWidth - 1);
			s_CursorY = WRAP(s_CursorY + yRel, 0, g_MapHeight - 1);
			s_CursorMoved = TRUE;
			s_CursorTimeout = CURSOR_MOVE_DELAY;
		}
	} else if ((2 == state) && (s_CursorTimeout == 0)) {
		s_CursorX = WRAP(s_CursorX + xRel, 0, g_MapWidth - 1);
		s_CursorY = WRAP(s_CursorY + yRel, 0, g_MapHeight - 1);
		s_CursorMoved = TRUE;
	} else {
		s_CursorTimeout = MAX(s_CursorTimeout - 1, 0);
	}
}

static uint8 noArrowKeysPressed() {
	return (
		(0 == Keyboard_Read(Key_Up)) &&
		(0 == Keyboard_Read(Key_Down)) &&
		(0 == Keyboard_Read(Key_Left)) &&
		(0 == Keyboard_Read(Key_Right))
	);
}


static void insertEntityAtCursor() {
	Entity e = { 0 };
	// Log("Inserting entity %i", s_CursorClass);
	e.class    = s_CursorClass;
	e.sprite = (Sprite*)malloc(sizeof(Sprite));
	e.sprite->x = s_CursorX + .5f;
	e.sprite->y = s_CursorY + .5f;
	Game_InitEntity(e);
}

static void deleteEntityAtCursor() {
	Entity* e;
	int i;
	for (i = 0; i < g_EntityCount; i++) {
		e = &g_Entities[i];
		if (((int)e->sprite->x == s_CursorX) &&
			((int)e->sprite->y == s_CursorY))
			Entities_RemoveByID(i);
	}
}

static void drawCursorClass() {
	const int x = 319 - 16 - 64;
	const int y = 200 - 64;

	SpriteRenderer_DrawImage(Game_GetDefaultImage(s_CursorClass), x, y);
}


static void updateAndDrawCursor() {
	// Update position
	tryMoveCursor(Key_Left, -1, 0);
	tryMoveCursor(Key_Right, 1, 0);
	tryMoveCursor(Key_Up, 0, -1);
	tryMoveCursor(Key_Down, 0, 1);
	s_CursorMovedPrev = s_CursorMoved;
	s_CursorMoved = FALSE;
	if (noArrowKeysPressed()) {
		// The user shouldn't be hindered when pressing keys rapidly
		s_CursorTimeout = 0;
		s_CursorMovedPrev = FALSE;
	}

	// Update currently selected wall type
	if (1 == Keyboard_Read(Key_Q)) {
		s_CursorWallType = WRAP(s_CursorWallType - 1, 0, 12);
	}
	if (1 == Keyboard_Read(Key_E)) {
		s_CursorWallType = WRAP(s_CursorWallType + 1, 0, 12);
	}

	if (Keyboard_Read(Key_Spacebar)) {
		// Set the wall at cursor, but make sure the edges will never be set to
		// 0, as this causes undefined behaviour and so might crash the game.
		if (s_CursorWallType == 0) {
			if (s_CursorX > 0 && s_CursorY > 0 && s_CursorX < g_MapWidth && s_CursorY < g_MapHeight)
				g_Map[g_MapWidth * s_CursorY + s_CursorX] = s_CursorWallType;
		} else {
			g_Map[g_MapWidth * s_CursorY + s_CursorX] = s_CursorWallType;
		}
	}

	// Draw cursor and it's current wall type
	GFX_PSet(2*s_CursorX  , 2*s_CursorY  , COLOR_CURSOR);
	GFX_PSet(2*s_CursorX+1, 2*s_CursorY+1, COLOR_CURSOR);
	GFX_RectFilled(
		319 - 16, 199 - 16, 319, 199,
		s_CursorWallType ? (s_CursorWallType << 4) : 0
	);
}

static void drawPlayer() {
	GFX_PSet(
		(int)(g_Player.x * 2 + .5f),
		(int)(g_Player.y * 2 + .5f),
		((s_PlayerFlash++) % 2 == 0) ? 12 : 0
	);
}

static void drawEntities() {
	int i;
	Entity* e;
	for (i = 0; i < g_EntityCount; i++) {
		e = &g_Entities[i];
		GFX_PSet(
			(int)(e->sprite->x * 2 + .5f),
			(int)(e->sprite->y * 2 + .5f),
			COLOR_ENTITY
		);
	}

}




void Editor_Init() {

}

void Editor_Cleanup() {

}


void Editor_Tick() {

	//Keyboard_DebugShowKeys();
	if (Keyboard_Read(Key_Enter) == PRESSED)
		g_EditorActive ^= TRUE;
	if (!g_EditorActive) return;

	// Actions
	if (Keyboard_Read(Key_Minus) == PRESSED)
		dialogLoadMap();
	if (Keyboard_Read(Key_Equals) == PRESSED)
		dialogSaveMap();
	if (Keyboard_Read(Key_LBracket) == PRESSED)
		s_CursorClass = WRAP(s_CursorClass - 1, 0, CLASS_COUNT - 1);
	if (Keyboard_Read(Key_RBracket) == PRESSED)
		s_CursorClass = WRAP(s_CursorClass + 1, 0, CLASS_COUNT - 1);
	if (Keyboard_Read(Key_Z) == PRESSED)
		insertEntityAtCursor();
	if (Keyboard_Read(Key_Del) == PRESSED)
		deleteEntityAtCursor();

	// Draw
	drawMapPreview();
	updateAndDrawCursor();
	drawCursorClass();
	drawEntities();
	drawPlayer();
}
