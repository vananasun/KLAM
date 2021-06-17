#ifndef __G_ACTOR_H__
#define __G_ACTOR_H__
#include "E.H"
#include "E_ENTITY.H"
#include "E_ASTAR.H"

//
// Contains all the structs and global function declarations for each of
// the actors.
//

typedef struct {
	AStar astar;
	uint8 pathTicks;
} Reisen;
void Reisen_Init(Entity* _);
void Reisen_Cleanup(Entity* e);
uint8 Reisen_Update(Entity* e);


typedef struct {
	uint8 unused;
} Scrotox;
void Scrotox_Init(Entity* e);
uint8 Scrotox_Update(Entity* e);

#endif
