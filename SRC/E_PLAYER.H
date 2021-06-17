#ifndef _E_PLAYER_H_
#define _E_PLAYER_H_

typedef struct {
	float x;
	float y;
	float dirX;
	float dirY;
	float planeX;
	float planeY;
	float xspeed;
	float yspeed;
	float turnVelocity;

	// pre-calculations
	float _invDet;
} Player;

//void Player_Init(int angle = 0);
void Player_Update(Player* player, float frameTime);


extern Player g_Player;

#endif
