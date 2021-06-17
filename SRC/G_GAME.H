#ifndef __G_GAME_H__
#define __G_GAME_H__
#include "E_ENTITY.H"



/***
 *
 * In-game objects
 *
 */
typedef enum {
	ENTITY_PAAL = 0,
	ENTITY_REISEN,
	ENTITY_SCROTOX,

	CLASS_COUNT
} EntityClass;

// @TODO: implement editor sprites




/***
 *
 * Sprites
 *
 **/
#define IMAGE_NUM 3
#define SPR_PAAL 1
#define SPR_REISENS0 2
#define SPR_SCROTOXS0 3
extern const char IMAGE_PATHS[IMAGE_NUM][27];

/**
 * @param {uint16} class - EntityClass
 *
 * @returns {uint16} The image ID for the given class
 */
uint16 Game_GetDefaultImage(uint16 class);

/**
 * @param {Entity} e - "class" and "sprite" fields are expected to be
 *                     already set by now.
 *                     EXPECTED TO ALSO BE INITIALIZED TO ZERO!!!
 */
void Game_InitEntity(Entity e);

/**
 * Game initialization hook.
 */
void Game_Init();

#endif
