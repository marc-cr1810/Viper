#include "complexobject.h"

//
//
//		Methods
//
//

static void complex_dealloc(ViComplexObject *self)
{
	Vi_TYPE(self)->tp_free((ViObject *)self);
}

ViTypeObject ViComplexType = {
	VAROBJECT_HEAD_INIT(&ViComplexType, 0)	// base
	"float",								// tp_name
	"Float object type",					// tp_doc
	sizeof(ViComplexObject),				// tp_size
	0,										// tp_itemsize
	TPFLAGS_DEFAULT | TPFLAGS_BASETYPE,		// tp_flags
	(destructor)complex_dealloc,			// tp_dealloc
	0,										// tp_number_methods
	0,										// tp_sequence_methods
	0,										// tp_clear
	0,										// tp_base
	0,										// tp_dict
	0,										// tp_new
	Mem_Free								// tp_free
};

ViObject *ViComplexObject_FromComplex(ViComplex cval)
{
	ViComplexObject *obj = ViObject_NEW(ViComplexObject, &ViComplexType);
	obj->ob_cval = cval;
	return (ViObject *)obj;
}

ViObject *ViComplexObject_FromDoubles(double real, double imag)
{
	ViComplex c;
	c.real = real;
	c.imag = imag;
	return ViComplexObject_FromComplex(c);
}
