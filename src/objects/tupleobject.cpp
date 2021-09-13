#include "tupleobject.h"

#include "../core/error.h"

//
//
//		Methods
//
//

static void tuple_dealloc(ViTupleObject *self)
{
	Vi_size_t i;
	// Untrack list for GC
	// begin trashcan
	if (self->ob_items != NULL)
	{
		i = Vi_SIZE(self);
		while (--i >= 0)
		{
			ViObject_DECREF(self->ob_items[i]);
		}
		Mem_Free(self->ob_items);
	}
	Vi_TYPE(self)->tp_free((ViObject *)self);

	// end trashcan
}

static Vi_size_t tuple_length(ViTupleObject* tuple)
{
	return Vi_SIZE(tuple);
}

static ViObject* tuple_item(ViTupleObject* tuple, Vi_size_t i)
{
	if (i < 0 || i >= Vi_SIZE(tuple))
	{
		ViError_SetString(ViExc_IndexError, "tuple index out of range");
		std::cout << "" << std::endl;
		return NULL;
	}
	ViObject_INCREF(tuple->ob_items[i]);
	return tuple->ob_items[i];
}

static ViSequenceMethods tuple_sequence_methods = {
	(lenfunc)tuple_length,		// sq_length
	0,	// sq_concat
	0,	// sq_repeat
	(sizeargfunc)tuple_item,	// sq_item
	0,	// sq_slice
	0,	// sq_assign_item
	0,	// sq_assign_slice
	0,	// sq_contains
	0,	// sq_inplace_concat
	0,	// sq_inplace_repeat
};

ViTypeObject ViTupleType = {
	VAROBJECT_HEAD_INIT(&ViTupleType, 0)	// base
	"tuple",								// tp_name
	"Tuple object type",					// tp_doc
	sizeof(ViTupleObject),					// tp_size
	0,										// tp_itemsize
	TPFLAGS_DEFAULT | TPFLAGS_BASETYPE |	// tp_flags
		TPFLAGS_TUPLE_SUBCLASS,
	(destructor)tuple_dealloc,				// tp_dealloc
	0,										// tp_number_methods
	&tuple_sequence_methods,				// tp_sequence_methods
	0,										// tp_clear
	&ViBaseObjectType,						// tp_base
	0,										// tp_dict
	0,										// tp_new
	Mem_Free								// tp_free
};

ViObject* ViTupleObject_New(Vi_size_t size)
{
	ViTupleObject* obj;
	size_t alloc;

	obj = ViObject_NEW(ViTupleObject, &ViTupleType);
	if (obj == NULL)
		return NULL;
	if (size == 0)
	{
		obj->ob_items = NULL;
		alloc = 0;
	}
	else
	{
		alloc = size * sizeof(ViObject*);
		obj->ob_items = (ViObject**)Mem_Alloc(alloc);
	}
	VAROBJECT_SET_SIZE(obj, size);

	return (ViObject*)obj;
}

ViObject* ViTupleObject_FromArray(ViObject* const* src, Vi_size_t size)
{
	if (size == 0)
		return ViTupleObject_New(0);

	ViTupleObject* tuple = (ViTupleObject*)ViTupleObject_New(size);
	if (tuple == NULL)
		return NULL;

	ViObject** dest = tuple->ob_items;
	for (Vi_size_t i = 0; i < size; i++)
	{
		ViObject* item = src[i];
		ViObject_INCREF(item);
		dest[i] = item;
	}
	return (ViObject*)tuple;
}
