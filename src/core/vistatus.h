#ifndef __VISTATUS_H__
#define __VISTATUS_H__

#include "../port.h"

typedef struct _vistatus
{
	enum _type {
		ViStatus_TYPE_OK = 0,
		ViStatus_TYPE_ERROR = 1,
		ViStatus_TYPE_EXIT = 2
	} type;
	const char* func;
	const char* error_msg;
	int exit_code;
} ViStatus;

ViStatus ViStatus_Ok();
ViStatus ViStatus_Error(const char* error_msg);
ViStatus ViStatus_NoMemory();
ViStatus ViStatus_Exit(int exit_code = 0);

bool ViStatus_IsError(ViStatus status);
bool ViStatus_IsExit(ViStatus status);
bool ViStatus_Exception(ViStatus status);

#endif // __VISTATUS_H__