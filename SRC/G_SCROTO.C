#include "E.H"
#include "E_SPRITE.H"
#include "G_ACTOR.H"


void Scrotox_Init(Entity* e) {
	e->update = (UPDATEFUNC)Scrotox_Update;
	//e->sprite.imageId = SPR_REISENS0;
}


uint8 Scrotox_Update(Entity* e) {
	UNUSED_PARAMETER(e);
	Log("Scrotox update");
	return FALSE;
}
