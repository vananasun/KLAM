#include "E.H"
#include "E_SPRITE.H"
#include "E_ASTAR.H"
#include "E_PLAYER.H"
#include "G_GAME.H"
#include "G_ACTOR.H"
#include <stdlib.h>

#define MOVE_SPEED 0.016f;
#define TRACE_FREQUENCY 50 /* How many ticks before calculating path to player */


volatile float g_DTP = 0;

float sqrtf(float f) {
    const int32 result = 0x1fbb4000 + (*(int32*)&f >> 1);
    return *(float*)&result;   
}

float calcDistance(float x0, float y0, float x1, float y1) {
	float hor, vert;
	hor = x1 - x0;
	vert = y1 - y0;
	return sqrtf((hor*hor) + (vert*vert));
}


void Reisen_Init(Entity* e) {
	Reisen* _ = malloc(sizeof(Reisen));

	e->update = (UPDATEFUNC)Reisen_Update;
	e->cleanup = (CLEANUPFUNC)Reisen_Cleanup;
	e->sprite->imageId = SPR_REISENS0;

	e->_size = sizeof(Reisen);
	e->_ = _;
	AStar_Create(&_->astar);
	_->pathTicks = TRACE_FREQUENCY;
}

void Reisen_Cleanup(Entity* e) {
	AStar_Cleanup(&((Reisen*)e->_)->astar);
	SAFE_DELETE_PTR(e->_);
}


uint8 Reisen_Update(Entity* e) {
	float toX,toY,spdX,spdY,dist;
	Reisen* reisen = (Reisen*)e->_;
	
	if (--reisen->pathTicks == 0) {
		AStar_Trace(
			&reisen->astar,
			e->sprite->x,
			e->sprite->y,
			g_Player.x,
			g_Player.y
		);
		reisen->pathTicks = TRACE_FREQUENCY;
	}

	spdX = (reisen->astar.path[0].x + .5f) - e->sprite->x;
	spdY = (reisen->astar.path[0].y + .5f) - e->sprite->y;
	dist = sqrtf((spdX * spdX) + (spdY * spdY));
	g_DTP = (1.0f / calcDistance(e->sprite->x, e->sprite->y, g_Player.x, g_Player.y) * 9.0f);
	
	if (calcDistance(e->sprite->x, e->sprite->y, g_Player.x, g_Player.y) < 0.5f) {
		// If close enough to the player then hurt the player
		// Log("Reisen hit player");
	} else {
		// Move across path
		e->sprite->x += (spdX / dist) * MOVE_SPEED;
		e->sprite->y += (spdY / dist) * MOVE_SPEED;
	}

	return FALSE;
}
