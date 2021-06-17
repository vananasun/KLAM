#ifndef _E_RENDER_H_
#define _E_RENDER_H_

#include "E_PART.H"

//extern float far g_ZBuffer[320];
extern int far g_WallHeights[320];

void Renderer_Init();
void Renderer_Cleanup();
void Renderer_UpdateCache();
void Renderer_RenderWalls();
void Renderer_RenderInstances();
void Renderer_RenderParticles();

#endif
