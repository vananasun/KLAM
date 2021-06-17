#ifndef _E_PART_H
#define _E_PART_H

#include "E.H"

#define SNOWFLAKE_COUNT 255
#define MAX_PARTICLES (SNOWFLAKE_COUNT + 255)

typedef struct {
	float x;
	float y;
	float z;
	float xspeed;
	float yspeed;
	float zspeed;
	float friction;
	uint8 color;
	uint8 radius;
	uint8 alive;
} Particle;

extern Particle far g_Particles[MAX_PARTICLES];


void ParticleManager_Reset();
void ParticleManager_Cleanup();
void ParticleManager_Update();
void ParticleManager_InitSnow();
void ParticleManager_RemoveAll();
void ParticleManager_RenderAll();

void Particle_Create(Particle* part);
void Particle_Update(Particle* part);

#endif
