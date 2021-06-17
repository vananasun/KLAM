#include "E.H"
#include "E_PART.H"
#include "E_MAP.H"
#include "E_PLAYER.H"
#include "E_RENDER.H"
#include "E_GFX.H"
#include <stdlib.h>
#include <string.h>

#define RND(x) ((float)rand()/(float)(RAND_MAX/(x)))


// @TODO: Optimize looping by re-sorting alive first, whilst preserving
//        Z-index (set to current particle count)
Particle far g_Particles[MAX_PARTICLES];


static void generateSnowflake(Particle* pPart) {
	pPart->x         = g_Player.x + (RND(14) - 7.0f);
	pPart->y         = g_Player.y + (RND(14) - 7.0f);
	pPart->z         = 3;
	pPart->xspeed    = 0.0f;//RND(0.5f) - 0.5f;
	pPart->yspeed    = 0.0f;//RND(0.5f) - 0.5f;
	pPart->zspeed    = -.05f * (1.0f + RND(1.5f));
	pPart->friction  = 0.93f;
	pPart->color     = 15;
	pPart->radius    = 16;
	pPart->alive     = TRUE;
}



void ParticleManager_Update() {

	// Render snowflakes
	Particle part;
	int i;

	for (i = 0; i < SNOWFLAKE_COUNT; i++) {

		if (!g_Particles[i].alive) continue;
		part = g_Particles[i];

		part.x += part.xspeed;
		part.y += part.yspeed;
		part.z += part.zspeed;
		part.xspeed *= part.friction;
		part.yspeed *= part.friction;

		if (part.z < -0.97f) {
			// Snow particle must lay on ground for some time after which it
			// will be destroyed and regenerated.
			if (part.z < -1.02f) {
				generateSnowflake(&g_Particles[i]);
				continue;
			} else {
				part.zspeed = -0.001f;
				part.radius = MAX(2, part.radius - 1);
			}

		}

		g_Particles[i] = part;

	}
}

void ParticleManager_Cleanup() {

}

void ParticleManager_InitSnow() {
	int i;
	for (i = 0; i < SNOWFLAKE_COUNT; i++) {
		generateSnowflake(&g_Particles[i]);
		g_Particles[i].z = RND(4);
	}
}

void ParticleManager_Reset() {
	memset(g_Particles, 0, sizeof(Particle) * MAX_PARTICLES);
}

void ParticleManager_RenderAll() {

	// Cached variables
	uint8 cachedColor;
	int16 cachedYOffset;
	float cachedCamX   = g_Player.x;
	float cachedCamY   = g_Player.y;
	float cachedDirX   = g_Player.dirX;
	float cachedDirY   = g_Player.dirY;
	float cachedPlaneX = g_Player.planeX;
	float cachedPlaneY = g_Player.planeY;
	float cachedInvDet = g_Player._invDet;

	// Particle variables
	Particle part;
	float particleX, particleY, particleZ, particleHeightHalf, transX, transY;

	// Draw variables
	int i, drawStartX, drawStartY, drawEndX, drawEndY, screenX;

	for (i = 0; i < MAX_PARTICLES; i++) {
		if (!g_Particles[i].alive) continue;

		part = g_Particles[i];
		cachedColor = part.color;

		// Translate relative to camera
		particleX = part.x - cachedCamX;
		particleY = part.y - cachedCamY;

		// Transform with inverse camera matrix
		transY = cachedInvDet * (-cachedPlaneY * particleX + cachedPlaneX * particleY);
		if (transY < 0.5f) continue; // frustum culling
		transX = cachedInvDet * (cachedDirY * particleX - cachedDirX * particleY);
		screenX = (int)(160 * (1.0f + transX / transY));

		// Calculate height of the particle on screen
		particleZ = (-200.0f * (part.z / transY));
		particleHeightHalf = ABS((int)(part.radius / transY)) / 2;

		// Calculate dimensions of particle on screen
		drawEndX = particleHeightHalf + screenX;
		if (drawEndX  <   0) continue;
		if (drawEndX >= 320) drawEndX = 320;
		drawEndY = particleHeightHalf + 100 + particleZ;
		if (drawEndY  <   0) continue;
		if (drawEndY >= 200) drawEndY = 199;

		drawStartX = -particleHeightHalf + screenX;
		if (drawStartX < 0) drawStartX = 0;
		drawStartY = -particleHeightHalf + 100 + particleZ;
		if (drawStartY < 0) drawStartY = 0;
		if (drawStartY >= 200) drawStartY = 199;


		__asm {
		// Loop through every vertical stripe of the sprite on screen
		//
		// AL: y iterator
		// AH: drawEndY
		// CX: x iterator
		// DX: drawEndX
		// DI: back buffer offset

		// Pre-calculate initial draw offset
		mov     ax, [drawStartY]
		mov     bx, ax
		shl     bx, 8
		shl     ax, 6
		add     ax, bx
		mov     [cachedYOffset], ax

		// Cache data into CPU regs
		mov     bx, BACK_BUFFER_SEG
		mov     es, bx
		mov     dx, [drawEndX]
		mov     ah, byte ptr [drawEndY] // mov     ah, [drawEndY]
		mov     cx, [drawStartX]

		forScreenX:
		cmp     cx, dx
		jnl     forScreenXDone

		// if (transY >= g_ZBuffer[x]) continue;
		/*asm push    ax
		asm mov     bx, cx
		asm shl     bx, 2
		asm fld     dword [g_ZBuffer + bx]
		asm fcomp   dword [transY]
		asm fnstsw  ax
		asm test    ah, 0x41
		asm pop     ax
		asm jne     forScreenYDone*/

		// Calculate offset at which to start drawing
		mov     bx, [cachedYOffset]
		mov     di, bx
		mov     di, bx
		add     di, cx

		mov     bl, [cachedColor]
		mov     al, byte ptr [drawStartY] // mov al, byte ptr [drawStartY]
		forScreenY:
		cmp     al, ah
		jnb     forScreenYDone
		mov     es:[di], bl
		add     di, 320
		inc     al
		jmp     forScreenY
		forScreenYDone:

		inc     cx
		jmp     forScreenX
		forScreenXDone:
		}//__asm

	}



	// // Cached variables
	// uint8 cachedColor;
	// int16 cachedYOffset;
	// float cachedCamX   = g_Player.x;
	// float cachedCamY   = g_Player.y;
	// float cachedDirX   = g_Player.dirX;
	// float cachedDirY   = g_Player.dirY;
	// float cachedPlaneX = g_Player.planeX;
	// float cachedPlaneY = g_Player.planeY;
	// float cachedInvDet = g_Player._invDet;

	// // Particle variables
	// Particle part;
	// float particleX, particleY, particleZ, particleHeightHalf, transX, transY;
	// //float invDet = 1.0f / (cachedPlaneX * cachedDirY - cachedDirX * cachedPlaneY);
	// // @TODO: cache invDet in player

	// // Draw variables
	// int i, drawStartX, drawStartY, drawEndX, drawEndY, screenX;


	// for (i = 0; i < MAX_PARTICLES; i++) {
	// 	if (!g_Particles[i].alive) continue;
	// 	part = g_Particles[i];
	// 	cachedColor = part.color;

	// 	// Translate relative to camera
	// 	particleX = part.x - cachedCamX;
	// 	particleY = part.y - cachedCamY;

	// 	// Transform with inverse camera matrix
	// 	transY = cachedInvDet * (-cachedPlaneY * particleX + cachedPlaneX * particleY);
	// 	if (transY < 0.5f) continue; // frustum culling
	// 	transX = cachedInvDet * (cachedDirY * particleX - cachedDirX * particleY);
	// 	screenX = (int)(160 * (1.0f + transX / transY));

	// 	// Calculate height of the particle on screen
	// 	particleZ = (-200.0f * (part.z / transY));
	// 	particleHeightHalf = ABS((int)(part.radius / transY)) / 2;

	// 	// Calculate dimensions of particle on screen
	// 	drawEndX = particleHeightHalf + screenX;
	// 	if (drawEndX  <   0) continue;
	// 	if (drawEndX >= 320) drawEndX = 320;
	// 	drawEndY = particleHeightHalf + 100 + particleZ;
	// 	if (drawEndY  <   0) continue;
	// 	if (drawEndY >= 200) drawEndY = 199;

	// 	drawStartX = -particleHeightHalf + screenX;
	// 	if (drawStartX < 0) drawStartX = 0;
	// 	drawStartY = -particleHeightHalf + 100 + particleZ;
	// 	if (drawStartY < 0) drawStartY = 0;
	// 	if (drawStartY >= 200) drawStartY = 199;



	// 	// Loop through every vertical stripe of the sprite on screen
	// 	//
	// 	// AL: y iterator
	// 	// AH: drawEndY
	// 	// CX: x iterator
	// 	// DX: drawEndX
	// 	// DI: back buffer offset

	// 	// Pre-calculate initial draw offset
	// 	asm mov     ax, word [drawStartY]
	// 	asm mov     bx, ax
	// 	asm shl     bx, 8
	// 	asm shl     ax, 6
	// 	asm add     ax, bx
	// 	asm mov     word [cachedYOffset], ax

	// 	// Cache data into CPU regs
	// 	asm mov     bx, BACK_BUFFER_SEG
	// 	asm mov     es, bx
	// 	asm mov     dx, word [drawEndX]
	// 	asm mov     ah, byte [drawEndY]
	// 	asm mov     cx, word [drawStartX]

	// 	forScreenX:
	// 	asm cmp     cx, dx
	// 	asm jnl     forScreenXDone:

	// 	// if (transY >= g_ZBuffer[x]) continue;
	// 	/*asm push    ax
	// 	asm mov     bx, cx
	// 	asm shl     bx, 2
	// 	asm fld     dword [g_ZBuffer + bx]
	// 	asm fcomp   dword [transY]
	// 	asm fnstsw  ax
	// 	asm test    ah, 0x41
	// 	asm pop     ax
	// 	asm jne     forScreenYDone*/

	// 	// Calculate offset at which to start drawing
	// 	asm mov     bx, word [cachedYOffset]
	// 	asm mov     di, bx
	// 	asm mov     di, bx
	// 	asm add     di, cx

	// 	asm mov     bl, byte [cachedColor]
	// 	asm mov     al, byte [drawStartY]
	// 	forScreenY:
	// 	asm cmp     al, ah
	// 	asm jnb     forScreenYDone
	// 	asm mov     es:[di], bl
	// 	asm add     di, 320
	// 	asm inc     al
	// 	asm jmp     forScreenY
	// 	forScreenYDone:

	// 	asm inc     cx
	// 	asm jmp     forScreenX
	// 	forScreenXDone:

	// }
}




void Particle_Create(Particle* part) {
	((void)part);
}

void Particle_Update(Particle* part) {
	((void)part);
}
