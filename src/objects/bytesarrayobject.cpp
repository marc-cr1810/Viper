#include "bytesarrayobject.h"

#include "../core/error.h"
#include "intobject.h"

static inline int valid_index(Vi_size_t i, Vi_size_t limit)
{
	return (size_t)i < (size_t)limit;
}

static int get_byte_value(ViObject* obj, int* value)
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
		ViError_SetString(ViExc_ValueError, "byte must be in range (0, 256)");
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

static void bytearray_dealloc(ViByteArrayObject *self)
{
	if (self->ob_bytes != 0)
		Mem_Free(self->ob_bytes);
	Vi_TYPE(self)->tp_free((ViObject *)self);
}

static Vi_size_t bytearray_length(ViByteArrayObject* bytearray)
{
	return Vi_SIZE(bytearray);
}

static ViObject* bytearray_item(ViByteArrayObject* bytearray, Vi_size_t i)
{
	if (valid_index(i, Vi_SIZE(bytearray)))
	{
		ViError_SetString(ViExc_IndexError, "byte array index out of range");
		return NULL;
	}
	return ViIntObject_FromInt(bytearray->ob_bytes[i]);
}

static int bytearray_assign_item(ViByteArrayObject* bytearray, Vi_size_t i, ViObject* value)
{
	int ival;
	if (valid_index(i, Vi_SIZE(bytearray)))
	{
		ViError_SetString(ViExc_IndexError, "byte array index out of range");
		return -1;
	}

	if (value == NULL)
	{
		ViError_SetString(ViExc_TypeError, "value cannot be a null object");
		return -1;
	}

	if (!get_byte_value(value, &ival))
		return -1;

	bytearray->ob_bytes[i] = ival;
	return 0;
}

static ViSequenceMethods bytearray_sequence_methods = {
	(lenfunc)bytearray_length,				// sq_length
	0,	// sq_concat
	0,	// sq_repeat
	(sizeargfunc)bytearray_item,			// sq_item
	0,	// sq_slice
	(sizeobjargproc)bytearray_assign_item,	// sq_assign_item
	0,	// sq_assign_slice
	0,	// sq_contains
	0,	// sq_inplace_concat
	0,	// sq_inplace_repeat
};

ViTypeObject ViByteArrayType = {
	VAROBJECT_HEAD_INIT(&ViByteArrayType, 0)	// base
	"string",									// tp_name
	"String object type",						// tp_doc
	sizeof(ViByteArrayObject),					// tp_size
	0,											// tp_itemsize
	TPFLAGS_DEFAULT | TPFLAGS_BASETYPE,			// tp_flags
	(destructor)bytearray_dealloc,				// tp_dealloc
	0,											// tp_number_methods
	&bytearray_sequence_methods,				// tp_sequence_methods
	0,											// tp_clear
	0,											// tp_base
	0,											// tp_dict
	0,											// tp_new
	Mem_Free									// tp_free
};

ViObject* ViByteArrayObject_FromString(const char* bytes, size_t size)
{
	ViByteArrayObject* obj;
	size_t alloc;

	obj = ViObject_NEW(ViByteArrayObject, &ViByteArrayType);
	if (obj == NULL)
		return NULL;
	if (size == 0)
	{
		obj->ob_bytes = NULL;
		alloc = 0;
	}
	else
	{
		alloc = size + 1;
		obj->ob_bytes = (Vi_int8_t*)Mem_Alloc(alloc);
		if (bytes != NULL && size > 0)
			memcpy(obj->ob_bytes, bytes, size);
		obj->ob_bytes[size] = '\0'; // Trailing NULL byte (end of string)
	}
	VAROBJECT_SET_SIZE(obj, size);
	obj->ob_alloc = alloc;
	return (ViObject*)obj;
}
