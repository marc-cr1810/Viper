#ifndef __CODEOBJECT_H__
#define __CODEOBJECT_H__

#include "object.h"

typedef struct _codeobject
{
	ViObject_HEAD
	ViObject* co_name; 		// Name (for reference)
	ViObject* co_filename;	// The file where it was loaded from
	int co_argcount;		// Amount of arguments
	int co_localvarcount;	// Amount of local variables
	ViObject* co_code;		// Instruction opcodes
	ViObject* co_consts;	// List of constants used
	ViObject* co_names;		// List of names used
	ViObject* co_varnames;	// List of local variable names used
	Vi_int32_t co_lineno;	// The line number where the source first occured
} ViCodeObject;

extern ViTypeObject ViCodeType;

/* Create a new empty code object with the source location */
ViCodeObject* ViCodeObject_NewEmpty(const char* filename, const char* func_name, Vi_int32_t lineno);

#endif // __CODEOBJECT_H__