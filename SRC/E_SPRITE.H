#ifndef __SPRITE_H__
#define __SPRITE_H__

#define MAX_IMAGES 500
#define MAX_SPRITES 200

typedef struct {
	uint16 imageId;
	float x;
	float y;
} Sprite;

typedef struct {
	uint16 width;
	uint16 height;
	uint8* far pixels;
} Image;

extern Sprite* g_Sprites[MAX_SPRITES];


/**
 * Loads an image file for later use.
 * You may only use converted image files.
 *
 * @param {const char*} filename
 *
 * @returns {uint16} Image ID for use in sprite object, 0 on failure.
 */
uint16 SpriteRenderer_LoadImage(const char* filename);

/**
 * Removes every sprite instance in the world.
 */
void SpriteRenderer_RemoveInstances();

/**
 * Removes a single sprite instance from the world by swapping around pointers.
 *
 * @param {Sprite*} ptr - Pointer used to identify the sprite.
 */
void SpriteRenderer_Remove(Sprite* ptr);

/**
 * Inserts a sprite instance for rendering.
 *
 * @param {Sprite*} spr
 */
void SpriteRenderer_Insert(Sprite* spr);

/**
 * Renders all (3D) sprites in the world on the screen.
 */
void SpriteRenderer_RenderAll();

/**
 * Draws an image on the screen in 2D.
 *
 * @param {uint8} imageId - ID returned by SpriteRenderer_LoadImage
 * @param {int}   x       - X coordinate
 * @param {int}   y       - Y coordinate
 */
void SpriteRenderer_DrawImage(uint16 imageId, int x, int y);
#endif
