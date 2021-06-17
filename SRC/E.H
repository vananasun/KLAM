#ifndef _UM_H_
#define _UM_H_

#ifdef __GNUC__
#define far /**/
#elif __WATCOMC__
#define far __far
#define near __near
#define interrupt __interrupt
#endif

// Types
typedef unsigned char   uint8;
typedef signed   char   int8;
typedef unsigned int    uint16;
typedef signed   int    int16;
typedef unsigned long   uint32;
typedef signed   long   int32;
#define TRUE  1
#define FALSE 0


// Macro functions
#define ABS(x) (((x)<0)?-(x):(x))
#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))
#define CLAMP(x, min, max) (((x)<(min))?(min):(((x)>(max))?(max):(x)))
#define WRAP(x, min, max) (((x)<(min))?(max):(((x)>(max))?(min):(x)))
#define SAFE_DELETE_PTR(p) {if((p))free((p));}
#define SAFE_DELETE_FAR_PTR(p) {if((p))_ffree((p));}
#define UNUSED_PARAMETER(x) ((void)(x))
#define SGN(x) ((x>0)?1:((x<0)?-1:0))


// Global variables
extern uint8 g_Running;


// Global functions
void Error(const char* format, ...); // stops the program
void Log(const char* format, ...); // writes to in-game console

#endif
