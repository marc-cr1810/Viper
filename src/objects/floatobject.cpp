#include "floatobject.h"

//
//
//		Methods
//
//

static void float_dealloc(ViFloatObject *self)
{
	Vi_TYPE(self)->tp_free((ViObject *)self);
}

ViTypeObject ViFloatType = {
	VAROBJECT_HEAD_INIT(&ViFloatType, 0) // base
	"float",							 // tp_name
	"Float object type",				 // tp_doc
	sizeof(ViFloatObject),				 // tp_size
	0,									 // tp_itemsize
	TPFLAGS_DEFAULT | TPFLAGS_BASETYPE,	 // tp_flags
	(destructor)float_dealloc,			 // tp_dealloc
	0,									 // tp_number_methods
	0,									 // tp_sequence_methods
	0,									 // tp_clear
	0,									 // tp_base
	0,									 // tp_dict
	0,									 // tp_new
	Mem_Free							 // tp_free
};

ViObject* ViFloatObject_FromDouble(double dval)
{
	ViFloatObject* obj = ViObject_NEW(ViFloatObject, &ViFloatType);
	obj->ob_fval = dval;
	return (ViObject*)obj;
}
