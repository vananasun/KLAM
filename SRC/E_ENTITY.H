#ifndef __E_ENTITY_H__
#define __E_ENTITY_H__
#include "E.H"
#include "E_SPRITE.H"

#define MAX_ENTITIES 256


enum EntityType {
	UNINITIALIZED = 0, STATIC, AREA, ACTOR, NPC
};

typedef struct {
	uint16 class;
	uint8 type; // EntityType
	uint8 (*update)(void* me); // returns: destroy or not
	void (*cleanup)(void* me);
	uint16 _size;
	void* _; // inner struct
	Sprite* sprite;
} Entity;

typedef uint8(*UPDATEFUNC)(void*);
typedef void(*CLEANUPFUNC)(void*);

extern uint16 g_EntityCount;
extern Entity g_Entities[MAX_ENTITIES];

void Entities_Reset();
void Entities_Update();
uint8 Entity_NoUpdate();
void Entity_NoCleanup();

Entity* Entities_Insert(Entity e);
void Entities_RemoveByID(uint16 id);

#endif


