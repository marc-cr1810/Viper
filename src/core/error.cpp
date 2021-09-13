#include "error.h"

#include "runtime.h"
#include "viperrun.h"

#include "../objects/stringobject.h"

/* Forward declarations */

static void error_set_object(ViThreadState *tstate, ViObject *exception, ViObject *value);

/* Helper functions */

static inline ViObject *error_occured(ViThreadState *tstate)
{
	assert(tstate != NULL);
	return tstate->curr_exc_type;
}

static void error_restore(ViThreadState *tstate, ViObject *type, ViObject *value)
{
	ViObject *oldtype, *oldvalue;

	oldtype = tstate->curr_exc_type;
	oldvalue = tstate->curr_exc_value;

	tstate->curr_exc_type = type;
	tstate->curr_exc_value = value;

	ViObject_XDECREF(oldtype);
	ViObject_XDECREF(oldvalue);
}

static void error_clear(ViThreadState *tstate)
{
	error_restore(tstate, NULL, NULL);
}

static ViObject *error_format_string(ViThreadState *tstate, ViObject *exception, const char *string)
{
	if (ViException_Check(exception))
		return NULL;

	ViObject *result = NULL;
	ViObject *str1;
	ViObject *str2;

	str1 = ((ViExceptionObject *)exception)->type;
	if (str1 == NULL)
		return NULL;
	str2 = ViStringObject_FromString(string);
	if (str2 == NULL)
		return NULL;

	ViObject *sep_str = ViStringObject_FromString(": ");

	result = (ViObject *)ViString_Concat(ViString_Concat(str1, sep_str), str2);
	ViObject_XDECREF(str1);
	ViObject_XDECREF(str2);
	return result;
}

static ViObject *error_format(ViThreadState *tstate, ViObject *exception, const char *string)
{
	error_clear(tstate);
	ViObject *str = error_format_string(tstate, exception, string);
	error_set_object(tstate, exception, str);
	ViObject_XDECREF(str);
	return NULL;
}

static void error_set_object(ViThreadState *tstate, ViObject *exception, ViObject *value)
{
	if (exception == NULL || ViException_Check(exception))
	{
		error_format(tstate, ViExc_SystemError, "exception is not an Exception object");
		return;
	}

	ViObject_XINCREF(value);
	error_restore(tstate, exception, value);
}

static void error_print_exception(ViThreadState *tstate)
{
	if (error_occured(tstate))
	{
		ViStringObject *msg = (ViStringObject *)tstate->curr_exc_value;
		printf(msg->ob_svar);

		Vi_Exit(((ViExceptionObject *)tstate->curr_exc_type)->exitcode);
	}
}

/* API Function definitions */

void ViError_SetNone(ViObject *exception)
{
	ViThreadState *tstate = ViThreadState_GET();
	error_set_object(tstate, exception, (ViObject *)NULL);
}

void ViError_SetObject(ViObject *exception, ViObject *value)
{
	ViThreadState *tstate = ViThreadState_GET();
	error_set_object(tstate, exception, value);
}

void ViError_SetString(ViObject *exception, const char *string)
{
	ViThreadState *tstate = ViThreadState_GET();
	error_format(tstate, exception, string);
}

ViObject *ViError_Occurred()
{
	ViThreadState *tstate = ViThreadState_GET();
	return error_occured(tstate);
}

void ViError_Clear()
{
	ViThreadState *tstate = ViThreadState_GET();
	error_clear(tstate);
}

void ViError_Restore(ViObject *type, ViObject *value)
{
	ViThreadState *tstate = ViThreadState_GET();
	error_restore(tstate, type, value);
}

void ViError_Format(ViObject *exception, const char *string)
{
	ViThreadState *tstate = ViThreadState_GET();
	error_format(tstate, exception, string);
}

void ViError_Print()
{
	ViThreadState *tstate = ViThreadState_GET();
	error_print_exception(tstate);
}

void ViError_NoMemory()
{
	ViThreadState *tstate = ViThreadState_GET();
	if (ViExc_MemoryError == NULL)
	{
		std::cout << "out of memory" << std::endl;
		return;
	}
	ViObject *msg = ViStringObject_FromString("out of memory");
	error_set_object(tstate, ViExc_MemoryError, msg);
}

void ViError_BadInternalCall()
{
	ViThreadState *tstate = ViThreadState_GET();
	ViObject *msg = ViStringObject_FromString("bad argument to internal function");
	error_set_object(tstate, ViExc_SystemError, msg);
}

ViObject *ViExceptionObject_New(const char *type, int exitcode)
{
	ViExceptionObject *exc = ViObject_NEW(ViExceptionObject, &ViExceptionType);
	if (exc == NULL)
		return NULL;

	exc->type = ViStringObject_FromString(type);
	exc->context = NULL;
	exc->exitcode = exitcode;

	return (ViObject *)exc;
}

ViTypeObject ViExceptionType = {
	VAROBJECT_HEAD_INIT(&ViExceptionType, 0)	// base
	"exception",								// tp_name
	"Exception object type",				    // tp_doc
	sizeof(ViExceptionObject),					// tp_size
	0,										    // tp_itemsize
	TPFLAGS_DEFAULT | TPFLAGS_BASETYPE |	    // tp_flags
		TPFLAGS_BASE_EXC_SUBCLASS,
	0,										    // tp_dealloc
	0,										    // tp_number_methods
	0,                  					    // tp_sequence_methods
	0,										    // tp_clear
	0,										    // tp_base
	0,										    // tp_dict
	0,										    // tp_new
	Mem_Free								    // tp_free
};

ViObject *ViExc_Exception = ViExceptionObject_New("Exception", 1);
ViObject *ViExc_TypeError = ViExceptionObject_New("TypeError", 2);
ViObject *ViExc_IndexError = ViExceptionObject_New("IndexError", 3);
ViObject *ViExc_ValueError = ViExceptionObject_New("ValueError", 4);

ViObject *ViExc_SyntaxError = ViExceptionObject_New("SyntaxError", 5);
ViObject *ViExc_IndentationError = ViExceptionObject_New("IndentationError", 6);
ViObject *ViExc_TabError = ViExceptionObject_New("TabError", 7);

ViObject *ViExc_KeyboardInterrupt = ViExceptionObject_New("KeyboardInterrupt", 8);
ViObject *ViExc_MemoryError = ViExceptionObject_New("MemoryError", 9);
ViObject *ViExc_SystemError = ViExceptionObject_New("SystemError", 10);
ViObject *ViExc_RuntimeError = ViExceptionObject_New("RuntimeError", 11);