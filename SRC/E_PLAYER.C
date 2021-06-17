#include "E_PLAYER.H"
#include "E_KEYBD.H"
#include <math.h>


extern uint8* far g_Map;
extern uint16 g_MapWidth;
extern uint16 g_MapHeight;


void Player_Update(Player* p, float frameTime) {
	float oldDirX, oldPlaneX;
	float moveSpeed = frameTime * .0040f;
	float turnSpeed = frameTime * .0100f;

	if (Keyboard_Read(Key_W) || Keyboard_Read(Key_Up)) {
		p->xspeed += p->dirX * moveSpeed;
		p->yspeed += p->dirY * moveSpeed;
	}

	if (Keyboard_Read(Key_S) || Keyboard_Read(Key_Down)) {
		p->xspeed -= p->dirX * moveSpeed;
		p->yspeed -= p->dirY * moveSpeed;
	}

	if (Keyboard_Read(Key_A)) {
		p->xspeed += p->dirY * moveSpeed;
		p->yspeed -= p->dirX * moveSpeed;
	}

	if (Keyboard_Read(Key_D)) {
		p->xspeed -= p->dirY * moveSpeed;
		p->yspeed += p->dirX * moveSpeed;
	}

	// Smooth turning
	if (Keyboard_Read(Key_Left)) {
		p->turnVelocity -= turnSpeed;
	}
	if (Keyboard_Read(Key_Right)) {
		p->turnVelocity += turnSpeed;
	}
	oldDirX = p->dirX;
	p->dirX = p->dirX * cos(p->turnVelocity) - p->dirY * sin(p->turnVelocity);
	p->dirY = oldDirX * sin(p->turnVelocity) + p->dirY * cos(p->turnVelocity);
	oldPlaneX = p->planeX;
	p->planeX = p->planeX * cos(p->turnVelocity) - p->planeY * sin(p->turnVelocity);
	p->planeY = oldPlaneX * sin(p->turnVelocity) + p->planeY * cos(p->turnVelocity);
	p->turnVelocity *= 0.7f; // @TODO: pow frameTime

	// Pre-calculate invDet
	p->_invDet = 1.0f / (p->planeX * p->dirY - p->dirX * p->planeY);





	// Move
	// @TODO: collision checking with map and nearby objects
	if (!g_Map[g_MapWidth * (int)p->y + (int)(p->x + p->xspeed)]) {
		p->x += p->xspeed;
	}
	if (!g_Map[g_MapWidth * (int)(p->y + p->yspeed) + (int)p->x]) {
		p->y += p->yspeed;
	}
	p->xspeed *= 0.68f; // @TODO: To the power of 1 / frameTime
	p->yspeed *= 0.68f;





}
