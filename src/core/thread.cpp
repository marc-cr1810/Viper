#include "thread.h"

#include "vimem.h"
#include "interpreter.h"

ViThreadState* ViThreadState_New(_interpreterstate* interp)
{
	ViThreadState* tstate = (ViThreadState*)Mem_Alloc(sizeof(ViThreadState));
	if (tstate == NULL)
		return NULL;

	tstate->interp = interp;

	tstate->curr_exc_type = NULL;
	tstate->curr_exc_value = NULL;

	interp->thread = tstate;

	return tstate;
}
