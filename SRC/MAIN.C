#include "E.H"
#include "E_GFX.H"
#include "E_CMD.H"
#include "E_CLOCK.H"
#include "E_KEYBD.H"
#include "E_RENDER.H"
#include "E_PLAYER.H"
#include "E_EDITOR.H"
#include "E_ENTITY.H"
#include "E_SPRITE.H"
#include "E_PART.H"
#include "E_FONT.H"
#include "E_MAP.H"
#include "G_GAME.H"
#include "TESTS.H"
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <dos.h>

#define LOG_MAX_LINES 5
#define LOG_TIMEOUT 280
#define LOG_LINE_LEN 320 / 8 + 1
static uint8 s_LogLines = 0;
static char s_Log[LOG_MAX_LINES][LOG_LINE_LEN] = { 0 };
static int s_LogTimeout = 0;

Player g_Player;
uint8 g_Running = TRUE;



static void popLogLine() {
	int line;
	for (line = 0; line < s_LogLines; line++) {
		memcpy(s_Log[line], s_Log[line + 1], LOG_LINE_LEN);
	}
	s_LogLines--;
}

void Log(const char* format, ...) {

	va_list args;
	char* lineStr;

	// Pop older lines to make room for new ones.
	if (s_LogLines >= LOG_MAX_LINES) {
		popLogLine();
	}

	// Format the message into the appropriate line
	lineStr = s_Log[s_LogLines];
	memset(lineStr, 0, LOG_LINE_LEN);
	va_start(args, format);
	vsprintf(lineStr, format, args);
	va_end(args);
	s_LogLines++;
	s_LogTimeout = LOG_TIMEOUT;
}

void Log_UpdateAndDraw() {

	int line;

	// Remove items from the log when timeout is due
	if (s_LogTimeout == 0) {
		if (s_LogLines > 0) {
			popLogLine();
			s_LogTimeout = LOG_TIMEOUT / 3 * 2;
		} else {
			s_LogTimeout = -1;
		}
	} else if (s_LogTimeout != -1) {
		s_LogTimeout--;
	}

	// Display log lines
	for (line = 0; line < s_LogLines; line++) {
		Print(
			2,
			199 - 8 - line * 10,
			9,
			s_Log[line]
		); // @TODO: print unformatted for speed
	}
}




void Cleanup() {
	Audio_Cleanup();
	ParticleManager_Cleanup();
	MapManager_Cleanup();
	Renderer_Cleanup();
	Clock_Cleanup();
	Keyboard_Remove();
	GFX_Cleanup();
}


void Error(const char* format, ...) {

	// Format the string (if one was given)
	static char internalStr[1024];
	va_list args;

	// Reset the GFX
	GFX_Cleanup();

	if (format) {
		memset(internalStr, 0, 1024);
		va_start(args, format);
		vsprintf(internalStr, format, args);
		va_end(args);
		printf("Oh noes! A error happen?!\n\"%s\"\n\n", internalStr);
	}
	printf("Press any key to continue.");

	// @TODO: Autosave a running game

	sleep(5);//Keyboard_WaitForKey();
	Cleanup();
	exit(0);
}



int main() {
	int s;
	float frameTime;

	// @TODO: Init player position and direction in Map Loader
	g_Player.x = 2.5f;
	g_Player.y = 1.5f;
	g_Player.dirX = -1.0f;
	g_Player.dirY = 0.0f;
	g_Player.planeX = 0.0f;
	g_Player.planeY = -1.0;//0.85f; // 0.66f

	

	GFX_Init();
	Font_LoadFromFile("assets/font/umbono.fnt");
	MapManager_LoadFromFile("assets/map/trunk01.map");
	
	if (Tests_Perform()) {
		Log_UpdateAndDraw();
		GFX_Blit();
		sleep(3);
		return 0; // quit if the tests want us to
	}

	for (s = 1; s < IMAGE_NUM; s++)
		SpriteRenderer_LoadImage(IMAGE_PATHS[s]);

	Renderer_Init();
	Audio_Init();
	Game_Init();
	Clock_Init();
	Keyboard_Install();


	while (!Keyboard_Read(Key_Esc) && g_Running) {

		frameTime = Clock_Tick();
		//frameTime = 0.05; // @TODO: fix timer after sound ISR setup

		if (!g_EditorActive) {
			ParticleManager_Update();
			Player_Update(&g_Player, frameTime);
		}
		Entities_Update();


		Renderer_RenderWalls();
		ParticleManager_RenderAll();
		SpriteRenderer_RenderAll();
		Editor_Tick();
		CMD_Update();

		Log_UpdateAndDraw();
		Clock_DrawFPS();
		GFX_Blit();
	}



	Cleanup();
	return 0;
}
