#include "intobject.h"

//
//
//		Methods
//
//

static void int_dealloc(ViIntObject *self)
{
	Vi_TYPE(self)->tp_free((ViObject *)self);
}

ViTypeObject ViIntType = {
	VAROBJECT_HEAD_INIT(&ViIntType, 0)	// base
	"int",								// tp_name
	"Interger object type",				// tp_doc
	sizeof(ViIntObject),				// tp_size
	0,									// tp_itemsize
	TPFLAGS_DEFAULT | TPFLAGS_BASETYPE, // tp_flags
	(destructor)int_dealloc,			// tp_dealloc
	0,									// tp_number_methods
	0,									// tp_sequence_methods
	0,									// tp_clear
	0,									// tp_base
	0,									// tp_dict
	0,									// tp_new
	Mem_Free							// tp_free
};

ViObject* ViIntObject_FromInt(Vi_int32_t ival)
{
	ViIntObject* obj = ViObject_NEW(ViIntObject, &ViIntType);
	obj->ob_ival = ival;
	return (ViObject*)obj;
}

ViObject *ViIntObject_FromString(const char *str, int base)
{
	ViIntObject *obj = ViObject_NEW(ViIntObject, &ViIntType);
	obj->ob_ival = std::stoi(str, nullptr, base);
	return (ViObject *)obj;
}
