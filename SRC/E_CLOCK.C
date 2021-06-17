#include "E.H"
#include "E_CLOCK.H"
#include "E_AUDIO.H"
#include "E_FONT.H"
#include <dos.h>
#include <stdio.h>
#include <string.h>

// @NOTE: Ticks wrapping around uint32 size might cause unwanted behaviour.
// @TODO: Fix FPS-counter

#define TIMER_DIVISOR(hz) ((uint32)( 1193180.0f / (1193180.0f / (float)(hz) ) ))‬

static int s_FrameCount = 0;
static int s_FPS = 0;
static uint32 s_TimeNextSecond = 0;
static uint32 s_Time = 0;
static float s_FrameTime = 1.0f;
void interrupt (*s_OldISR)();

static void interrupt timerISR() {
	// @TODO: Call sound mixer
	Audio_Mix();
	outp(0x20, 0x20);
}


void Clock_Init() {
	/*int timerDivisor = 0x0001;
	outp(0x43, 0x36);
	outp(0x40, timerDivisor & 0x00FF);
	outp(0x40, (timerDivisor >> 8) & 0x00FF);

	s_OldISR = getvect(0x1C);
	setvect(0x1C, timerISR);*/

	__asm {
	mov al, 36h
	out 43h, al
	mov al, 2ah
	out 40h, al
	mov al, 1h
	out 40h, al
	sti	
	}
	s_OldISR = _dos_getvect(0x1C);
	_dos_setvect(0x1C, timerISR);
}


void Clock_Cleanup() {
	__asm {
	cli
	mov al, 36h
	out 43h, al
	mov al, 0h
	out 40h, al
	mov al, 0h
	out 40h, al
	sti
	}
	_dos_setvect(0x1C, s_OldISR);
}




long Clock_GetTicks() {
	union REGS regs;
	long c, s, m, h;
	regs.h.ah = 0x2C;
	intdos(&regs, &regs);
	c = regs.h.dl;
	s = regs.h.dh;
	m = regs.h.cl;
	h = regs.h.ch;
	return (c + 100 * (s + 60 * (m + 60 * h) ) );
}


Time Clock_TicksToTime(long ticks) {
	Time time = { 0 };
	// @TODO: Implement
	return time;
}


float Clock_Tick() {
	uint32 oldTime;
	oldTime = s_Time;
	s_Time = Clock_GetTicks();
	s_FrameTime = (s_Time - oldTime) / CLOCK_RATE;
	// Log("%f",s_FrameTime);
	return s_FrameTime;
}


void Clock_DrawFPS() {
	char fpsStr[5];
	s_FrameCount++;
	//Log("%lu > %lu", s_Time, s_TimeNextSecond);
	if (s_Time > s_TimeNextSecond) {
		s_TimeNextSecond = s_Time + ((100/18.2f*CLOCK_RATE));
		s_FPS = s_FrameCount;
		s_FrameCount = 0;
	}
	sprintf(fpsStr, "%i", s_FPS);
	Print(319 - 16 - 0, 4, 0, fpsStr);
	Print(319 - 16 - 2, 2, 2, fpsStr);
}
