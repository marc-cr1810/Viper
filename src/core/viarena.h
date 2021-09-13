#ifndef __VIARENA_H__
#define __VIARENA_H__

#include "../port.h"
#include "../objects/object.h"

typedef struct _arena ViArena;

ViArena *ViArena_New();
void ViArena_Free(ViArena *arena);

void *ViArena_Alloc(ViArena *arena, size_t size);

int ViArena_AddViObject(ViArena *arena, ViObject *obj);

#endif // __VIARENA_H__