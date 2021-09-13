#include "object.h"

#include "../core/error.h"

static int type_is_subtype_chain(ViTypeObject* a, ViTypeObject* b)
{
	do {
		if (a == b)
			return 1;
		a = a->tp_base;
	} while (a != NULL);
		return (b == &ViBaseObjectType);
}

static void object_dealloc(ViObject* self)
{
	Vi_TYPE(self)->tp_free(self);
}

ViTypeObject ViBaseType = {
	VAROBJECT_HEAD_INIT(&ViBaseType, 0)		// base
	"type",									// tp_name
	"Base type",							// tp_doc
	sizeof(ViObject),						// tp_size
	0,										// tp_itemsize
	TPFLAGS_DEFAULT | TPFLAGS_BASETYPE,		// tp_flags
	0,										// tp_dealloc
	0,										// tp_number_methods
	0,										// tp_sequence_methods
	0,										// tp_clear
	0,										// tp_base
	0,										// tp_dict
	0,										// tp_new
	Mem_Free								// tp_free
};

ViTypeObject ViBaseObjectType = {
	VAROBJECT_HEAD_INIT(&ViBaseType, 0)		// base
	"object",								// tp_name
	"Base object type",						// tp_doc
	sizeof(ViObject),						// tp_size
	0,										// tp_itemsize
	TPFLAGS_DEFAULT | TPFLAGS_BASETYPE,		// tp_flags
	(destructor)object_dealloc,				// tp_dealloc
	0,										// tp_number_methods
	0,										// tp_sequence_methods
	0,										// tp_clear
	0,										// tp_base
	0,										// tp_dict
	0,										// tp_new
	Mem_Free								// tp_free
};

ViTypeObject ViNullType = {
	VAROBJECT_HEAD_INIT(&ViBaseType, 0)		// base
	"NullType",								// tp_name
	0,										// tp_doc
	0,										// tp_size
	0,										// tp_itemsize
	TPFLAGS_DEFAULT,						// tp_flags
	0,										// tp_dealloc
	0,										// tp_number_methods
	0,										// tp_sequence_methods
	0,										// tp_clear
	0,										// tp_base
	0,										// tp_dict
	0,										// tp_new
	Mem_Free								// tp_free
};

ViObject ViNullStruct = {
	1, &ViNullType
};

int ViType_IsSubtype(ViTypeObject* a, ViTypeObject* b)
{
	return type_is_subtype_chain(a, b);
}

int ViType_Ready(ViTypeObject *type)
{
	ViTypeObject *base, *dict;

	if (type->tp_flags & TPFLAGS_READY)
		return 0;

	assert((type->tp_flags & TPFLAGS_READYING) == 0);

	type->tp_flags |= TPFLAGS_READYING;

	if (type->tp_name == NULL)
	{
		ViError_Format(ViExc_SystemError, "type does not define the tp_name field");
		goto error;
	}

	// Initialize the tp_base (defaults to BaseObject unless thats's us)
	base = type->tp_base;
	if (base == NULL && type != &ViBaseObjectType)
	{
		base = &ViBaseObjectType;
		if (type->tp_flags & TPFLAGS_HEAPTYPE)
		{
			type->tp_base = (ViTypeObject *)ViObject_NEWREF((ViObject *)base);
		}
		else
		{
			type->tp_base = base;
		}
	}

	// Initlialize the base class
	if (base != NULL && base->tp_dict == NULL)
	{
		if (ViType_Ready(base) < 0)
			goto error;
	}

	// Initialize ob_type if NULL
	if (Vi_IS_TYPE(type, NULL) && base != NULL)
		ViObject_SET_TYPE(type, Vi_TYPE(base));
error:
	type->tp_flags &= ~TPFLAGS_READYING;
	return 0;
}

void ObjectNewRef(ViObject* obj)
{
	obj->ob_refcount = 1;
}

void ObjectDealloc(ViObject *obj)
{
	destructor dealloc = Vi_TYPE(obj)->tp_dealloc;
	(*dealloc)(obj);
}

ViObject *Object_NewRef(ViObject *obj)
{
	ViObject_INCREF(obj);
	return obj;
}

ViObject *Object_XNewRef(ViObject *obj)
{
	ViObject_XINCREF(obj);
	return obj;
}

ViObject* Object_New(ViTypeObject* type)
{
	ViObject* obj = (ViObject*)Mem_Alloc(type->tp_size);
	ObjectInit(obj, type);
	return obj;
}
