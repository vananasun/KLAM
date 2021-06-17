#include "E.H"
#include "E_SPRITE.H"
#include "E_ENTITY.H"
#include <stdlib.h>
#include <string.h>



uint16 g_EntityCount = 0;
Entity far g_Entities[MAX_ENTITIES];


// /**
//  * Re-sorts the entity array.
//  * All of the entities that are uninitialized will be pushed toward the
//  * back of the array, and the counter will be set appropriately.
//  * This is usually done after deletion of an entity.
//  */
// static void sortEntities() {


// }



uint8 Entity_NoUpdate(Entity* e) {
	UNUSED_PARAMETER(e);
	return FALSE;
}

void Entity_NoCleanup(Entity* e) {
	UNUSED_PARAMETER(e);
}



void Entities_Reset() {
	memset(g_Entities, 0, sizeof(g_Entities));
}

void Entities_Update() {
	int i;
	for (i = 0; i < g_EntityCount; i++) {
		if (g_Entities[i].update(&g_Entities[i])) {
			// Returned TRUE, so destroy the object.
			// @TODO: This might be a superfluous and archaic way of deletion
			Entities_RemoveByID(i);
		}
	}
}

Entity* Entities_Insert(Entity e) {
	if (g_EntityCount >= MAX_ENTITIES)
		return 0;
	memcpy(&g_Entities[g_EntityCount], &e, sizeof(Entity)); // copy struct
	// @TODO: Do sprite allocation here
	return &g_Entities[g_EntityCount++];
}

void Entities_RemoveByID(uint16 id) {

	g_Entities[id].cleanup(&g_Entities[id]);

	// If a sprite is associated with the entity, then GET RID OF IT.
	if (g_Entities[id].sprite != 0) {
		SpriteRenderer_Remove(g_Entities[id].sprite);
	}

	// Shift the entity array towards the beginning over the entity we want
	// to remove. Then uninitialize the last entity in the array because it
	// was duplicated by the shifting process.
	if (g_EntityCount > 1) {
		memcpy(
			&g_Entities[id],
			&g_Entities[id + 1],
			sizeof(Entity) * (g_EntityCount - id - 1)
		);
	}
	g_EntityCount--;

}
