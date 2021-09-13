#include "interpreter.h"

#include "vimem.h"
#include "error.h"
#include "runtime.h"

ViInterpreterState* ViInterpreter_New()
{
	ViInterpreterState* interp = (ViInterpreterState*)Mem_Alloc(sizeof(ViInterpreterState));
	if (interp == NULL)
		return NULL;

    interp->id_refcount = -1;

    _runtimestate *runtime = &ViRuntime;
    interp->runtime = runtime;

    ViConfig_InitViperConfig(&interp->config);

	struct _runtimestate::_interpreters *interpreters = &runtime->interpreters;

    if (interpreters->next_id < 0)
    {
        /* overflow or Py_Initialize() not called! */
        ViError_SetString(ViExc_RuntimeError, "failed to get an interpreter ID");
        Mem_Free(interp);
        interp = NULL;
    }
    else
    {
        interp->id = interpreters->next_id;
        interpreters->next_id += 1;
        interp->next = interpreters->head;
        if (interpreters->main == NULL)
            interpreters->main = interp;
        interpreters->head = interp;
    }

    if (interp == NULL)
        return NULL;

	return interp;
}

const ViConfig *ViInterpreterState_GetConfig(ViInterpreterState *interp)
{
    return &interp->config;
}
