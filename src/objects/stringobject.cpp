#include "stringobject.h"

#include "../core/error.h"

#include "intobject.h"

static inline int valid_index(Vi_size_t i, Vi_size_t limit)
{
	return (size_t)i < (size_t)limit;
}

static int get_char_value(ViObject* obj, int* value)
{
	if (!ViInt_Check(obj))
	{
		ViError_SetString(ViExc_TypeError, "object must be an integer type");
		*value = -1;
		return 0;
	}

	int v = ((ViIntObject*)obj)->ob_ival;
	if (v < 0 || v >= 256)
	{
		ViError_SetString(ViExc_ValueError, "char must be in range (0, 256)");
		*value = -1;
		return 0;
	}
	*value = v;
	return 1;
}

//
//
//		Methods
//
//

static void string_dealloc(ViStringObject *self)
{
	if (self->ob_svar != 0)
		Mem_Free(self->ob_svar);
	Vi_TYPE(self)->tp_free((ViObject *)self);
}

static Vi_size_t string_length(ViStringObject* string)
{
	return Vi_SIZE(string);
}

static ViObject* string_concat(ViStringObject* a, ViObject* b)
{
	ViStringObject* result = NULL;
	if (!ViString_CheckExact(b))
	{
		ViError_SetString(ViExc_TypeError, "can only concat strings");
		return NULL;
	}

	if (Vi_SIZE(a) == 0)
	{
		result = (ViStringObject*)b;
		ViObject_INCREF(result);
		return (ViObject*)result;
	}
	if (Vi_SIZE(b) == 0)
	{
		result = a;
		ViObject_INCREF(result);
		return (ViObject*)result;
	}

	result = (ViStringObject*)ViStringObject_FromStringAndSize(NULL, Vi_SIZE(a) + Vi_SIZE(b));
	if (result != NULL)
	{
		memcpy(result->ob_svar, a->ob_svar, Vi_SIZE(a));
		memcpy(result->ob_svar + Vi_SIZE(a), ((ViStringObject*)b)->ob_svar, Vi_SIZE(b));
	}
	return (ViObject*)result;
}

static ViObject* string_item(ViStringObject* string, Vi_size_t i)
{
	if (valid_index(i, Vi_SIZE(string)))
	{
		ViError_SetString(ViExc_IndexError, "string index out of range");
		return NULL;
	}
	return ViIntObject_FromInt(string->ob_svar[i]);
}

static int string_assign_item(ViStringObject* string, Vi_size_t i, ViObject* value)
{
	int ival;
	if (valid_index(i, Vi_SIZE(string)))
	{
		ViError_SetString(ViExc_IndexError, "string index out of range");
		return -1;
	}

	if (value == NULL)
	{
		ViError_SetString(ViExc_TypeError, "value cannot be a null object");
		return -1;
	}

	if (!get_char_value(value, &ival))
		return -1;

	string->ob_svar[i] = ival;
	return 0;
}

static ViSequenceMethods string_sequence_methods = {
	(lenfunc)string_length,				// sq_length
	(binaryfunc)string_concat,			// sq_concat
	0,	// sq_repeat
	(sizeargfunc)string_item,			// sq_item
	0,	// sq_slice
	(sizeobjargproc)string_assign_item,	// sq_assign_item
	0,	// sq_assign_slice
	0,	// sq_contains
	0,	// sq_inplace_concat
	0,	// sq_inplace_repeat
};

ViTypeObject ViStringType = {
	VAROBJECT_HEAD_INIT(&ViStringType, 0)	// base
	"string",								// tp_name
	"String object type",					// tp_doc
	sizeof(ViStringObject),					// tp_size
	0,										// tp_itemsize
	TPFLAGS_DEFAULT | TPFLAGS_BASETYPE,		// tp_flags
	(destructor)string_dealloc,				// tp_dealloc
	0,										// tp_number_methods
	&string_sequence_methods,				// tp_sequence_methods
	0,										// tp_clear
	&ViBaseObjectType,						// tp_base
	0,										// tp_dict
	0,										// tp_new
	Mem_Free								// tp_free
};

ViObject* ViStringObject_FromString(const char* bytes)
{
	return ViStringObject_FromStringAndSize(bytes, strlen(bytes));
}

ViObject* ViStringObject_FromStringAndSize(const char* bytes, Vi_size_t size)
{
	ViStringObject* obj;
	size_t alloc;

	if (size < 0)
	{
		ViError_SetString(ViExc_SystemError, "Negative size passed to ViStringObject_FromStringAndSize");
		return NULL;
	}

	obj = ViObject_NEW(ViStringObject, &ViStringType);
	if (obj == NULL)
		return NULL;
	if (size == 0)
	{
		obj->ob_svar = NULL;
		alloc = 0;
	}
	else
	{
		alloc = size + 1;
		obj->ob_svar = (char*)Mem_Alloc(alloc);
		if (bytes != NULL && size > 0)
			memcpy(obj->ob_svar, bytes, size);
		obj->ob_svar[size] = '\0'; // Trailing NULL byte (end of string)
	}
	VAROBJECT_SET_SIZE(obj, size);
	obj->ob_alloc = alloc;
	return (ViObject*)obj;
}

ViObject* ViString_Concat(ViObject* a, ViObject* b)
{
	if (!ViString_CheckExact(a))
	{
		ViError_SetString(ViExc_SystemError, "Tried to pass non-ViStringObject in ViString_Concat");
		return NULL;
	}

	return a->ob_type->tp_sequence_methods->sq_concat(a, b);
}

char *ViString_ToString(ViObject *str)
{
	if (!ViString_Check(str))
	{
		ViError_SetString(ViExc_TypeError, "type is not a string");
		return NULL;
	}
	return ((ViStringObject*)str)->ob_svar;
}
