#ifndef _E_CLOCK_H_
#define _E_CLOCK_H_
#include "E.H"

/** @var {float} CLOCK_RATE */
// #define CLOCK_RATE (1000.0f / 18.2f)
#define CLOCK_RATE ((float)210.9625220868595)

/** @type {Time} */
typedef struct {
    uint16 c;
    uint16 s;
    uint16 m;
    uint16 h;
} Time;


/**
 * Installs a high precision timer ISR.
 */
void Clock_Init();

/**
 * Restores the clock ISR to it's regular state.
 */
void Clock_Cleanup();

/**
 * @returns {long} Amount of current clock ticks.
 */
long Clock_GetTicks();

/**
 * @return {Time} CSMH struct.
 */
Time Clock_TicksToTime(long ticks);

/**
 * Update the frame time of the clock.
 *
 * @returns {float} frameTime
 */
float Clock_Tick();

/**
 * Show an FPS counter.
 */
void Clock_DrawFPS();




#endif
