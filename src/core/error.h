#ifndef __ERROR_H__
#define __ERROR_H__

#include "errorcode.h"
#include "../objects/object.h"

/* Error handling definitions */

void ViError_SetNone(ViObject *exception);
void ViError_SetObject(ViObject *exception, ViObject *value);
void ViError_SetString(ViObject *exception, const char *string);
ViObject *ViError_Occurred();
void ViError_Clear();
void ViError_Restore(ViObject *type, ViObject *value);
void ViError_Format(ViObject *exception, const char *string);

/* Convenience functions */

void ViError_Print();

void ViError_NoMemory();
void ViError_BadInternalCall();

/* Error objects */

/* Exception Type object */
extern ViTypeObject ViExceptionType;

/* EXCEPTION_HEAD defines the initial segment of every exception class. */
#define EXCEPTION_HEAD ViObject_HEAD ViObject* type; ViObject* context; int exitcode;

typedef struct _viexceptionobject
{
	EXCEPTION_HEAD
} ViExceptionObject;

ViObject *ViExceptionObject_New(const char *type, int exitcode);

#define ViException_Check(self) ViType_HasFeature((ViTypeObject*)self, TPFLAGS_BASE_EXC_SUBCLASS)

/* Predefined exceptions */
extern ViObject *ViExc_Exception;
extern ViObject *ViExc_TypeError;
extern ViObject *ViExc_IndexError;
extern ViObject *ViExc_ValueError;

extern ViObject *ViExc_SyntaxError;
extern ViObject *ViExc_IndentationError;
extern ViObject *ViExc_TabError;

extern ViObject *ViExc_KeyboardInterrupt;
extern ViObject *ViExc_MemoryError;
extern ViObject *ViExc_SystemError;
extern ViObject *ViExc_RuntimeError;

#endif // __ERROR_H__