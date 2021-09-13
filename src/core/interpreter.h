#ifndef __INTERPRETER_H__
#define __INTERPRETER_H__

#include "../port.h"
#include "thread.h"
#include "viconfig.h"
#include "../objects/object.h"

struct _runtimestate; // Defined in "runtime.h"

typedef struct _interpreterstate
{
	struct _interpreterstate *next;
	_runtimestate* runtime;

	int64_t id;
	int64_t id_refcount;
	int requires_idref;

	ViThreadState* thread;

	ViConfig config;
} ViInterpreterState;

ViInterpreterState* ViInterpreter_New();

const ViConfig *ViInterpreterState_GetConfig(ViInterpreterState *interp);

#endif // __INTERPRETER_H__