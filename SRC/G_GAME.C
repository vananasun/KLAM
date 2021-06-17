#include "E.H"
#include "E_SPRITE.H"
#include "G_GAME.H"
#include "G_ACTOR.H"


const char IMAGE_PATHS[IMAGE_NUM][27] = {
	"", // invalid image
	"assets/sprite/paal.spr",    // 1
	"assets/sprite/reis_s0.spr", // 2
};






static uint8 getTypeOfClass(uint16 class);
static void initStatic(Entity* e);
static void initActor(Entity* e);
//static void initArea(Entity* e);
//static void initNPC(Entity* e);


static uint8 getTypeOfClass(uint16 class) {

	// @TODO: Later on maybe do this with some enum of #define magic?
	//        Alternatively, put this in the G_GAME.H header.

	switch (class) {
	case ENTITY_REISEN:
	case ENTITY_SCROTOX:
		return ACTOR;

	default: return STATIC; // default to static entity (for now?)

	}

}


static void initStatic(Entity* e) {
	switch (e->class) {
	case ENTITY_PAAL:
		e->sprite->imageId = SPR_PAAL;
		break;
	}

	// Entity does not nothing
	e->update = (UPDATEFUNC)Entity_NoUpdate;
	e->cleanup = (CLEANUPFUNC)Entity_NoCleanup;

	if (e->sprite) SpriteRenderer_Insert(e->sprite);
}

static void initActor(Entity* e) {
	switch (e->class) {
	case ENTITY_REISEN:
		Reisen_Init(e);
		break;
	case ENTITY_SCROTOX:
		break;
	}


	if (e->sprite->imageId) {
		SpriteRenderer_Insert(e->sprite);
	}
}

/*static void initArea(Entity* e) {

}

static void initNPC(Entity* e) {

}
 */

static void debugDumpEntity(Entity e) {
	Log(
		"Ent: %i %i %i %i %i",
		(int)e.class,
		(int)e.type,
		(int)e._size,
		(int)e._,
		(int)e.sprite
	);
}


void Game_InitEntity(Entity e) {
	Entity* eStored;
	e.type = getTypeOfClass(e.class);
	eStored = Entities_Insert(e);
	if (0 == eStored) {
		Log("Too many entities");
		return;
	}


	switch (eStored->type) {
	case STATIC: initStatic(eStored); break;
	case ACTOR: initActor(eStored); break;
	// case AREA: initArea(eStored); break;
	// case NPC: initNPC(eStored); break;
	}
}


void Game_Init() {
	/* Entity paal = { 0 };
	paal.class = ENTITY_PAAL;
	paal.sprite.x = 2.5f;
	paal.sprite.y = 2.5f;
	Game_InitEntity(&paal); */
}

uint16 Game_GetDefaultImage(uint16 class) {
	switch (class) {
	case ENTITY_PAAL: return SPR_PAAL;
	case ENTITY_REISEN: return SPR_REISENS0;
	case ENTITY_SCROTOX: return SPR_SCROTOXS0;
	default: return 0;
	}
}
