#include "runtime.h"

ViStatus ViRuntimeState_Init(ViRuntimeState& runtime)
{
	runtime.interpreters.next_id = 0;

	runtime.initialized = 1;

	return ViStatus_Ok();
}

void ViRuntimeState_Finalize(ViRuntimeState& runtime)
{
	if (!runtime.initialized)
		return;

	Mem_Free(runtime.interpreters.main->thread);
	Mem_Free(runtime.interpreters.main);

	runtime.initialized = 0;
}

ViThreadState* ViRuntimeState_GetThreadState(ViRuntimeState* runtime)
{
	return runtime->interpreters.main->thread;
}

ViInterpreterState *ViRuntimeState_GetInterpreterState(ViRuntimeState *runtime)
{
	return runtime->interpreters.main;
}
