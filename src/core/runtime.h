#ifndef __RUNTIME_H__
#define __RUNTIME_H__

#include "../port.h"
#include "interpreter.h"
#include "vistatus.h"
#include "viconfig.h"

typedef struct _runtimestate
{
	int preinitializing; // Is running Vi_PreInitialize()?
	int preinitialized; // Is Viper preinitialized? Set to 1 by Vi_PreInitialize()?

	int core_initialized; // Is Viper core initialized? Set to 1 by Vi_InitializeCore()

	int initialized; // Is Viper initialized? Set to 1 by Vi_Initialize()

	struct _interpreters
	{
		ViInterpreterState *head;
		ViInterpreterState *main;
		int64_t next_id;
	} interpreters;

	ViPreConfig preconfig;
} ViRuntimeState;

extern ViRuntimeState ViRuntime;

ViStatus ViRuntimeState_Init(ViRuntimeState& runtime);
void ViRuntimeState_Finalize(ViRuntimeState& runtime);

ViThreadState* ViRuntimeState_GetThreadState(ViRuntimeState* runtime);
inline ViThreadState* ViThreadState_GET()
{
	return ViRuntimeState_GetThreadState(&ViRuntime);
}

ViInterpreterState *ViRuntimeState_GetInterpreterState(ViRuntimeState *runtime);
inline ViInterpreterState *ViInterpreterState_GET()
{
	return ViRuntimeState_GetInterpreterState(&ViRuntime);
}

#endif // __RUNTIME_H__